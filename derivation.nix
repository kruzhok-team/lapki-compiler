{ lib, python3Packages, arduino-cli, cargo, maturin, rustc, rustPlatform, fetchFromGitHub }:
with python3Packages;

let

pydantic-core = buildPythonPackage rec {
  pname = "pydantic_core";
  pyproject = true;
  version = "2.16.3";
  src = python.pkgs.fetchPypi {
    inherit pname version;
    sha256 = "sha256-HKxon4Cjq6stPABIsp7qV1ERQFTwMqlBoy3kyFLFnK0=";
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
    hash = "sha256-w1uC9tlMDOi1+qsE+U1OZkGxezPONofeG2H9wLFp5hk=";
  };
  doCheck = false;
 propagatedBuildInputs = with pkgs; [ 
    typing-extensions
  ];
  rustVersion = "latest";
};

pydantic = buildPythonPackage rec {
  pname = "pydantic";
  version = "2.6.3";
  src = python.pkgs.fetchPypi {
    inherit pname version;
    sha256 = "sha256-4HgFxMf1xoJuM6HUydR5UNfq80ho4mkPhZTS4wJB8R8=";
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

cyberiadaml-py = buildPythonPackage rec {
  pname = "cyberiadaml_py";
  version = "1.0";
  pyproject = true;
  src = python.pkgs.fetchPypi {
    inherit pname version;
    sha256 = "sha256-HTTk0WuOtVNYWniHd7gzSwsOfv0CtMYBcXTijDr+eME=";
  };
  doCheck = false;
  propagatedBuildInputs = with pkgs; [
    xmltodict
    poetry
    poetry-core
    pydantic
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
  propagatedBuildInputs = with pkgs; [typing-inspect docstring-parser];
};


aiofile = buildPythonPackage rec {
  pname = "aiofile";
  version = "3.8.7";
  src = python.pkgs.fetchPypi {
    inherit pname version;
    sha256 = "sha256:a8f9dec17282b3583337c4ef2d1a67f33072ab80dd03608041ed9e71b88dc521";
  };
  doCheck = false;
  propagatedBuildInputs = with pkgs; [caio];
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
  propagatedBuildInputs = with pkgs; [  
    attrs
    multidict
    async-timeout
    yarl
    frozenlist
    aiosignal
    aiodns
    brotli
    charset-normalizer
];
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
  LAPKI_COMPILER_SERVER_PORT = "8082"; 
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
