[![CI](https://github.com/notiel/fullgraphmlparser/actions/workflows/CI.yml/badge.svg)](https://github.com/notiel/fullgraphmlparser/actions/workflows/CI.yml)

This repository contains scripts allowing to generate C++ code from the State Machine diagram
(represented by graphml file) and vice versa.

## Commands

### Generating C++ code from State Machine diagram
```
python graphmltoqm state_machine_file_name.graphml
```
will generate all cpp/h files in the same folder as `state_machine_file_name.graphml` file.