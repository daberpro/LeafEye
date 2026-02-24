# LeafEye
Simple native application to detect multi-object durian leaf disease.

LeafEye is a native Windows desktop application developed using C++/WinRT and WinUI 3 (Windows App SDK). It utilizes a deep learning model to perform multi-object detection on durian leaves, classifying various diseases locally. The application relies on an offline-first architecture powered by ObjectBox to manage user data, scan histories, and detection results without requiring an active internet connection.

## Architecture & Features

* **Deep Learning Integration:** Processes image data to detect and classify multiple bounding-box instances of leaf diseases in a single execution.
* **Native Frontend:** Built on WinUI 3 with C++/WinRT projections. UI updates are handled reactively using `IObservableVector` combined with `INotifyPropertyChanged` for minimal rendering overhead.
* **Local Storage:** Integrates ObjectBox (C++ API) for NoSQL data persistence. Database transactions are isolated on background worker threads to maintain main-thread fluidity.
* **Concurrency:** Heavily utilizes C++ coroutines (`co_await`, `winrt::fire_and_forget`) and the `DispatcherQueue` to handle asynchronous data fetching and safe UI thread marshalling.
* **Data Models:** Relational schema handling User credentials, Profile metadata, batch Scan History, and individual File History (tracking confidence scores and disease IDs).
* **Application Telemetry:** Implements a custom, mutex-locked file logger to capture runtime execution states and global unhandled exceptions to `%LOCALAPPDATA%`.

## Tech Stack

* **UI Framework:** WinUI 3 / Windows App SDK
* **Language:** C++17 / C++/WinRT
* **Database:** ObjectBox
* **Markup:** XAML

## Prerequisites

To compile and run this application locally, the following development environment is required:

* Visual Studio 2022 (v17.0 or later)
* "Desktop development with C++" workload
* Windows App SDK C++ Templates component
* Windows 10 SDK (10.0.19041.0 or later)

## Build Instructions

1. Clone the repository:
   
   ```bash
   git clone https://github.com/daberpro/LeafEye.git
   ```
