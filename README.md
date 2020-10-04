# pipeline

`pipeline` is a library that lets you setup data processsing pipelines like this:

```cpp
// auto get_camera_ids = fn([] { return std::make_tuple(0, 1, 2, 3); });
// 
// auto get_frame = fn([](size_t index) -> cv::Mat { /* get frame from camera */ });
// auto preprocess = fn([](auto&& frame) { /* preprocess frame */ });
// auto predict_depth = fn([](auto&& frame) { /* predict depth */ });
//
// auto process_results = fn([](auto&& depth_1, auto&& depth_2, auto&& depth_3, auto&& depth_4) { /* process results */ });

auto depth_ml_pipeline = get_frame | preprocess | predict_depth;

auto pipeline = get_camera_ids | unzip_into(depth_ml_pipeline) | process_results;

pipeline();
```