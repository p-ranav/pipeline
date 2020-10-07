<p align="center">
  <img height="70" src="img/logo.png"/>  
</p>

## Getting Started

`pipeline` is a `C++17` header-only template library that lets you setup data processsing pipelines. Simply include `<pipeline/pipeline.hpp>` and you're good to go. There is also a single_include version in `single_include/`.

## Building Samples

```bash
git clone https://github.com/p-ranav/pipeline
cd pipeline
mkdir build && cd build
cmake -DPIPELINE_SAMPLES=ON ..
make
```

## Generating Single Header

```bash
python3 utils/amalgamate/amalgamate.py -c single_include.json -s .
```

## Contributing
Contributions are welcome, have a look at the [CONTRIBUTING.md](CONTRIBUTING.md) document for more information.

## License
The project is available under the [MIT](https://opensource.org/licenses/MIT) license.
