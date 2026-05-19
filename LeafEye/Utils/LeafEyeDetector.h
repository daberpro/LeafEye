#pragma once

#include <winrt/Windows.Foundation.h>
#include <onnxruntime_cxx_api.h>
#include <dml_provider_factory.h>
#include <vector>
#include <string>
#include <memory>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <filesystem>
#include <windows.h>

// ------------------------------------------------------------------
// Important: This detector assumes that onnxruntime.dll is pre‑loaded
// by the application BEFORE WinUI initialises. Add the following code
// to your App::App() constructor:
//
//   WCHAR exePath[MAX_PATH];
//   GetModuleFileNameW(nullptr, exePath, MAX_PATH);
//   std::filesystem::path appDir = std::filesystem::path(exePath).parent_path();
//   std::wstring dllPath = (appDir / L"onnxruntime.dll").wstring();
//   LoadLibraryW(dllPath.c_str());
//
// If you don't do this, the app may crash due to a DLL version conflict.
// ------------------------------------------------------------------

namespace winrt::LeafEye::Utils
{
    struct DetectionResult {
        float x1, y1, x2, y2;
        float confidence;
        int class_id;
    };

    struct Anchor {
        float cx, cy, w, h;
    };

    class LeafEyeDetector
    {
    private:
        HMODULE m_hOrtDll = nullptr;          // handle to our leafeye_onnxruntime.dll
        const OrtApi* m_api = nullptr;         // API table from our DLL

        OrtEnv* m_env = nullptr;
        OrtSession* m_session = nullptr;
        OrtMemoryInfo* m_memory_info = nullptr;
        OrtAllocator* m_allocator = nullptr;   // optional, for future GPU copy

        std::vector<Anchor> m_anchors;

        const int64_t m_batch_size = 1;
        const int64_t m_channels = 3;
        const int64_t m_height = 256;
        const int64_t m_width = 256;
        const int m_num_classes = 6; // 1 background + 5 diseases

        // Helper to convert OrtStatus to string and release it
        std::string GetOrtError(OrtStatus* status) {
            if (!status) return "No error info.";
            const char* msg = m_api->GetErrorMessage(status);
            std::string err(msg ? msg : "Unknown");
            m_api->ReleaseStatus(status);
            return err;
        }

        // Retrieve ORT API from our pre‑loaded leafeye_onnxruntime.dll
        const OrtApi* LoadOrtApiFromDll() {
            // Our DLL is already loaded by App::App() – get its handle
            HMODULE hDll = GetModuleHandleW(L"leafeye_onnxruntime.dll");
            if (!hDll)
                throw std::runtime_error(
                    "leafeye_onnxruntime.dll not loaded. "
                    "Make sure you pre‑load it in App::App().");

            auto OrtGetApiBase = reinterpret_cast<const OrtApiBase * (*)()>(
                GetProcAddress(hDll, "OrtGetApiBase"));
            if (!OrtGetApiBase)
                throw std::runtime_error("OrtGetApiBase not found in leafeye_onnxruntime.dll.");
            const OrtApiBase* apiBase = OrtGetApiBase();
            if (!apiBase)
                throw std::runtime_error("OrtGetApiBase returned null.");
            return apiBase->GetApi(ORT_API_VERSION);
        }

    public:
        LeafEyeDetector(const std::wstring& model_path)
        {
            // 1. Get the ORT API from our isolated DLL (never from the system DLL)
            m_hOrtDll = GetModuleHandleW(L"leafeye_onnxruntime.dll");
            m_api = LoadOrtApiFromDll();
            if (!m_api) throw std::runtime_error("Failed to initialize ORT API.");

            // 2. Environment
            OrtStatus* status = m_api->CreateEnv(ORT_LOGGING_LEVEL_WARNING, "LeafEyeEnv", &m_env);
            if (status) throw std::runtime_error("CreateEnv failed: " + GetOrtError(status));

            // 3. CPU memory info
            status = m_api->CreateCpuMemoryInfo(OrtArenaAllocator, OrtMemTypeDefault, &m_memory_info);
            if (status) throw std::runtime_error("CreateMemoryInfo failed: " + GetOrtError(status));

            // 4. Session options
            OrtSessionOptions* session_opts = nullptr;
            m_api->CreateSessionOptions(&session_opts);
            m_api->SetIntraOpNumThreads(session_opts, 1);
            m_api->SetSessionGraphOptimizationLevel(session_opts, ORT_ENABLE_ALL);

            // --- DirectML disabled for stability ---
            // To enable GPU acceleration in the future, uncomment the next block
            // and add GPU‑to‑CPU copying inside Detect().
            // status = OrtSessionOptionsAppendExecutionProvider_DML(session_opts, 0);
            // if (status) {
            //     OutputDebugStringA(("[DirectML] Not available – " + GetOrtError(status) + "\n").c_str());
            //     // Non‑fatal: session will run on CPU.
            // }
            // ----------------------------------------

            // 5. Build absolute path to the model file
            WCHAR exePath[MAX_PATH];
            GetModuleFileNameW(nullptr, exePath, MAX_PATH);
            std::filesystem::path appDir = std::filesystem::path(exePath).parent_path();
            std::filesystem::path modelFullPath = appDir / model_path;

            if (!std::filesystem::exists(modelFullPath)) {
                throw std::runtime_error("Model file not found: " +
                    std::string(modelFullPath.string().begin(), modelFullPath.string().end()));
            }

            status = m_api->CreateSession(m_env, modelFullPath.c_str(), session_opts, &m_session);
            m_api->ReleaseSessionOptions(session_opts);
            if (status)
                throw std::runtime_error("CreateSession failed: " + GetOrtError(status));

            // 6. Generate anchors
            GenerateAnchors();
        }

        ~LeafEyeDetector() {
            if (m_session)      m_api->ReleaseSession(m_session);
            if (m_memory_info)  m_api->ReleaseMemoryInfo(m_memory_info);
            if (m_env)          m_api->ReleaseEnv(m_env);
            // DO NOT FreeLibrary(m_hOrtDll) – we don’t own the module.
        }

        std::vector<DetectionResult> Detect(const std::vector<float>& input_tensor_values)
        {
            std::vector<int64_t> input_shape = { m_batch_size, m_channels, m_height, m_width };

            OrtValue* input_tensor = nullptr;
            OrtStatus* status = m_api->CreateTensorWithDataAsOrtValue(
                m_memory_info,
                const_cast<float*>(input_tensor_values.data()),
                input_tensor_values.size() * sizeof(float),
                input_shape.data(), input_shape.size(),
                ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT,
                &input_tensor);
            if (status) throw std::runtime_error("CreateTensor failed: " + GetOrtError(status));

            const char* input_names[] = { "input_image" };
            const char* output_names[] = { "pred_loc", "pred_conf", "aux_logits" };
            OrtValue* output_tensors[3] = {};

            status = m_api->Run(m_session, nullptr,
                input_names, &input_tensor, 1,
                output_names, 3, output_tensors);
            if (status) throw std::runtime_error("Run failed: " + GetOrtError(status));

            // Since DirectML is disabled, tensors are on CPU → direct access is safe
            float* pred_loc = nullptr;
            status = m_api->GetTensorMutableData(output_tensors[0], reinterpret_cast<void**>(&pred_loc));
            if (status) throw std::runtime_error("GetTensorMutableData failed: " + GetOrtError(status));

            float* pred_conf = nullptr;
            status = m_api->GetTensorMutableData(output_tensors[1], reinterpret_cast<void**>(&pred_conf));
            if (status) throw std::runtime_error("GetTensorMutableData failed: " + GetOrtError(status));

            // Number of anchors = (total elements in pred_loc) / 4
            OrtTensorTypeAndShapeInfo* shape_info = nullptr;
            status = m_api->GetTensorTypeAndShape(output_tensors[0], &shape_info);
            if (status) throw std::runtime_error(GetOrtError(status));

            size_t num_elements = 0;
            m_api->GetTensorShapeElementCount(shape_info, &num_elements);
            m_api->ReleaseTensorTypeAndShapeInfo(shape_info);

            size_t num_anchors = num_elements / 4;
            auto results = PostProcess(pred_loc, pred_conf, num_anchors);

            // Cleanup
            m_api->ReleaseValue(output_tensors[0]);
            m_api->ReleaseValue(output_tensors[1]);
            m_api->ReleaseValue(output_tensors[2]);
            m_api->ReleaseValue(input_tensor);

            return results;
        }

    private:
        // ================================================
        //  Anchor generation, post‑processing, NMS
        //  (unchanged from your original code)
        // ================================================
        void GenerateAnchors()
        {
            m_anchors.clear();
            std::vector<int> feature_maps = { 64, 32, 16, 8, 4 };
            std::vector<float> base_scales = { 0.05f, 0.15f, 0.35f, 0.6f, 0.85f };
            std::vector<float> aspect_ratios = { 1.0f, 2.0f, 0.5f };
            std::vector<float> scale_mults = { 1.0f, 1.26f, 1.58f };

            for (size_t k = 0; k < feature_maps.size(); ++k) {
                int fmap = feature_maps[k];
                float base_s = base_scales[k];

                for (int i = 0; i < fmap; ++i) {
                    for (int j = 0; j < fmap; ++j) {
                        float cx = (j + 0.5f) / fmap;
                        float cy = (i + 0.5f) / fmap;

                        for (float s_m : scale_mults) {
                            float s_k = std::min(base_s * s_m, 1.05f);
                            for (float ar : aspect_ratios) {
                                Anchor anc;
                                anc.cx = cx;
                                anc.cy = cy;
                                anc.w = s_k * std::sqrt(ar);
                                anc.h = s_k / std::sqrt(ar);
                                m_anchors.push_back(anc);
                            }
                        }
                    }
                }
            }
        }

        // =====================================================================
        // PostProcessing – softmax, decode, confidence filtering
        // =====================================================================
        std::vector<DetectionResult> PostProcess(
            float* pred_loc, float* pred_conf, size_t num_anchors)
        {
            std::vector<DetectionResult> candidates;
            const float conf_threshold = 0.30f;
            const float img_size = 256.0f;

            for (size_t i = 0; i < num_anchors; ++i) {
                // Softmax over classes at this anchor
                float max_logit = -1e10f;
                for (int c = 0; c < m_num_classes; ++c) {
                    max_logit = std::max(max_logit, pred_conf[i * m_num_classes + c]);
                }

                float sum_exp = 0.0f;
                std::vector<float> probs(m_num_classes);
                for (int c = 0; c < m_num_classes; ++c) {
                    probs[c] = std::exp(pred_conf[i * m_num_classes + c] - max_logit);
                    sum_exp += probs[c];
                }

                // Highest confidence among non‑background classes (c >= 1)
                float top_score = 0.0f;
                int top_label = 0;
                for (int c = 1; c < m_num_classes; ++c) {
                    float p = probs[c] / sum_exp;
                    if (p > top_score) {
                        top_score = p;
                        top_label = c;
                    }
                }

                // If confidence is high enough, decode the bounding box
                if (top_score >= conf_threshold) {
                    const Anchor& anc = m_anchors[i];

                    float dx = pred_loc[i * 4 + 0];
                    float dy = pred_loc[i * 4 + 1];
                    float dw = pred_loc[i * 4 + 2];
                    float dh = pred_loc[i * 4 + 3];

                    float decoded_cx = (dx * 0.1f) * anc.w + anc.cx;
                    float decoded_cy = (dy * 0.1f) * anc.h + anc.cy;
                    float decoded_w = std::exp(std::min(dw * 0.2f, 10.0f)) * anc.w;
                    float decoded_h = std::exp(std::min(dh * 0.2f, 10.0f)) * anc.h;

                    DetectionResult det;
                    det.class_id = top_label;
                    det.confidence = top_score;
                    det.x1 = std::max(0.0f, (decoded_cx - decoded_w / 2.0f) * img_size);
                    det.y1 = std::max(0.0f, (decoded_cy - decoded_h / 2.0f) * img_size);
                    det.x2 = std::min(img_size, (decoded_cx + decoded_w / 2.0f) * img_size);
                    det.y2 = std::min(img_size, (decoded_cy + decoded_h / 2.0f) * img_size);

                    // Remove tiny boxes (noise)
                    if ((det.x2 - det.x1) > 1.0f && (det.y2 - det.y1) > 1.0f) {
                        candidates.push_back(det);
                    }
                }
            }

            return ApplyNMS(candidates, 0.30f);
        }

        // =====================================================================
        // Intersection over Union
        // =====================================================================
        float CalculateIoU(const DetectionResult& a, const DetectionResult& b)
        {
            float x1 = std::max(a.x1, b.x1);
            float y1 = std::max(a.y1, b.y1);
            float x2 = std::min(a.x2, b.x2);
            float y2 = std::min(a.y2, b.y2);

            float intersection = std::max(0.0f, x2 - x1) * std::max(0.0f, y2 - y1);
            float area_a = (a.x2 - a.x1) * (a.y2 - a.y1);
            float area_b = (b.x2 - b.x1) * (b.y2 - b.y1);
            float union_area = area_a + area_b - intersection;

            return (union_area <= 0.0f) ? 0.0f : intersection / union_area;
        }

        // =====================================================================
        // Non‑Maximum Suppression
        // =====================================================================
        std::vector<DetectionResult> ApplyNMS(
            std::vector<DetectionResult>& boxes, float iou_threshold)
        {
            if (boxes.empty()) return {};

            // Sort by descending confidence
            std::sort(boxes.begin(), boxes.end(), [](const auto& a, const auto& b) {
                return a.confidence > b.confidence;
                });

            std::vector<DetectionResult> result;
            std::vector<bool> is_suppressed(boxes.size(), false);

            for (size_t i = 0; i < boxes.size(); ++i) {
                if (is_suppressed[i]) continue;
                result.push_back(boxes[i]);

                for (size_t j = i + 1; j < boxes.size(); ++j) {
                    if (is_suppressed[j]) continue;
                    if (CalculateIoU(boxes[i], boxes[j]) > iou_threshold) {
                        is_suppressed[j] = true;
                    }
                }
            }
            return result;
        }
    };
}