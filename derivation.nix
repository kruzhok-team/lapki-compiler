{ lib, python3Packages, arduino-cli, cargo, maturin, rustc, rustPlatform, fetchFromGitHub }:
with python3Packages;

let

aiologger = buildPythonPackage rec {
  pname = "aiologger";
  version = "0.7.0";
  src = python.pkgs.fetchPypi {
    inherit pname version;
    sha256 = "sha256-ek1ckbg2th6EKnkQcXhqPYDWtvpG+4/Y5zORJT7Lcqw=";
  };
  doCheck = false;
  propagatedBuildInputs = with pkgs; [
    aiofiles
  ];
};


aiopath = buildPythonPackage rec {
  pname = "aiopath";
  version = "0.6.11";
  src = python.pkgs.fetchPypi {
    inherit pname version;
    sha256 = "sha256-Lw1NkZUoFhLGUIy/oSrDGEwxVA0TueYhWjJYl9pZ3s0=";
  };
  doCheck = false;
  propagatedBuildInputs = with pkgs; [
    aiofile
    anyio
  ];
};

clang = buildPythonPackage rec {
  pname = "clang";
  version = "16.0.1.1";
  src = python.pkgs.fetchPypi {
    inherit pname version;
    sha256 = "sha256-TVkY9aiYXxV5RyOOGbYEOpP7X3e3qfKwH6+OlItEeKk=";
  };
  doCheck = false;
  propagatedBuildInputs = with pkgs; [ ];
}; 

pydantic-core = buildPythonPackage rec {
  pname = "pydantic_core";
  pyproject = true;
  version = "2.16.2";
  src = python.pkgs.fetchPypi {
    inherit pname version;
    sha256 = "sha256-C6UDhQ2LjcwYOR8Q3olq5R03/l/kPb+2o1xcXK0nGgY=";
  };
  nativeBuildInputs = [
    cargo
    rustPlatform.cargoSetupHook
    rustc
    rustPlatform.maturinBuildHook
  ];

  buildInputs = lib.optionals stdenv.isDarwin [ libiconv ];

  cargoDeps = rustPlatform.fetchCargoTarball {
    inherit src;
    name = "${pname}-${version}";
    hash = "sha256-qfkyies2NHhwWJ5ouBs0KASNcgi7WyazG5mNoMvAW1c=";
  };
  doCheck = false;
  propagatedBuildInputs = with pkgs; [ 
    typing-extensions
  ];
};

pydantic = buildPythonPackage rec {
  pname = "pydantic";
  version = "2.6.1";
  src = python.pkgs.fetchPypi {
    inherit pname version;
    sha256 = "4fd5c182a2488dc63e6d32737ff19937888001e2a6d86e94b3f233104a5d1fa9";
  };
  pyproject = true;
  doCheck = false;
  propagatedBuildInputs = with pkgs; [ 
    hatch-fancy-pypi-readme
    annotated-types
    pydantic-core
    typing-extensions
  ];
  nativeBuildInputs = [
      poetry-core
      hatchling
      maturin
  ];
};

cyberiadaml-py = buildPythonPackage rec {
  pname = "cyberiadaml_py";
  version = "1.1";
  pyproject = true;
  src = python.pkgs.fetchPypi {
    inherit pname version;
    sha256 = "2aa9eb0323ff8d22fcd603be874aa69cc90a3712ae97dcf1d72d3cfbfdc05dc3";
  };
  nativeBuildInputs = [
      poetry-core
  ];
  doCheck = false;
  propagatedBuildInputs = with pkgs; [
    pydantic
    xmltodict
  ];
};

typed-argument-parser = buildPythonPackage rec {
  pname = "typed-argument-parser";
  version = "1.10.0";
  src = python.pkgs.fetchPypi {
    inherit pname version;
    sha256 = "sha256-0ORjURVJ8pUd8paLGBuFbC4oG+GrH0RxQ33gjrQRPvI=";
  };
  doCheck = false;
  propagatedBuildInputs = with pkgs; [];
};

aiofile = buildPythonPackage rec {
  pname = "aiofile";
  version = "3.8.7";
  src = python.pkgs.fetchPypi {
    inherit pname version;
    sha256 = "sha256:a8f9dec17282b3583337c4ef2d1a67f33072ab80dd03608041ed9e71b88dc521";
  };
  doCheck = false;
  propagatedBuildInputs = with pkgs; [];
};

aiofiles = buildPythonPackage rec {
  pname = "aiofiles";
  version = "23.1.0";
  src = python.pkgs.fetchPypi {
    inherit pname version;
    sha256 = "sha256:edd247df9a19e0db16534d4baaf536d6609a43e1de5401d7a4c1c148753a1635";
  };
  doCheck = false;
  propagatedBuildInputs = with pkgs; [];
};

aiohttp = buildPythonPackage rec {
  pname = "aiohttp";
  version = "3.8.4";
  src = python.pkgs.fetchPypi {
    inherit pname version;
    sha256 = "sha256:bf2e1a9162c1e441bf805a1fd166e249d574ca04e03b34f97e2928769e91ab5c";
  };
  doCheck = false;
  propagatedBuildInputs = with pkgs; [];
};

argcomplete = buildPythonPackage rec {
  pname = "argcomplete";
  version = "3.4.0";
  pyproject = true;
  nativeBuildInputs = [
    setuptools
    setuptools-scm
  ];

  src = fetchFromGitHub {
    owner = "kislyuk";
    repo = pname;
    rev = "refs/tags/v${version}";
    hash = "sha256-4JMyBixc6LcSdpvvLmK4nyyqZMK2kuFcPU7OXhJLpoc=";
  };
  doCheck = false;
  propagatedBuildInputs = with pkgs; [];
};

in

buildPythonApplication {
  pname = "lapki-compiler";
  version = "0.2.0";
  pyproject = true;
  nativeBuildInputs = with pkgs; [
    poetry-core
  ];
  
  propagatedBuildInputs = [
    aiofile
    aiohttp
    aioshutil
    lxml
    pytest
    pytest-asyncio
    xmltodict
    aiologger
    aiopath
    clang
    pydantic
    arduino-cli
    argcomplete
    typed-argument-parser
    flake8
    cyberiadaml-py
  ];

  doCheck = false;

  src = ./.;
}

