#pragma once
#include "FileHistoryDetailDialog.g.h"

namespace winrt::LeafEye::implementation
{
    struct FileHistoryDetailDialog : FileHistoryDetailDialogT<FileHistoryDetailDialog>
    {
        FileHistoryDetailDialog(LeafEyeCore::FileHistoryModel const& model);

    private:
        LeafEyeCore::FileHistoryModel m_model{ nullptr };
    };
}
namespace winrt::LeafEye::factory_implementation
{
    struct FileHistoryDetailDialog : FileHistoryDetailDialogT<FileHistoryDetailDialog, implementation::FileHistoryDetailDialog>
    {
    };
}
