# pipeline

`pipeline` is a library that lets you setup data processsing pipelines like this:

```cpp
// auto get_frame = fn([](size_t index) -> cv::Mat { /* get frame from camera */ });
// auto preprocess = fn([](auto&& frame) { /* preprocess frame */ });
// auto predict_depth = fn([](auto&& frame) { /* predict depth */ });

auto depth_ml_pipeline = get_frame | preprocess | predict_depth;

auto get_camera_ids = fn([] { return std::make_tuple(0, 1, 2, 3); });

auto pipeline = get_camera_ids | unzip_into(depth_ml_pipeline) | process_results;

pipeline();
```