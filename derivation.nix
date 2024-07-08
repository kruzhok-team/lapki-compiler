{ lib, python3Packages, arduino-cli }:
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

cyberiadaml-py = buildPythonPackage rec {
  pname = "cyberiadaml-py";
  version = "1.1";
  src = python.pkgs.fetchPypi {
    inherit pname version;
    sha256 = "5e51d435783a6377c11cd24d1fe0a9ec3676232b773e5ea229dc257321ca505d";
  };
  doCheck = false;
  propagatedBuildInputs = with pkgs; [];
};

in

buildPythonApplication {
  pname = "lapki-compiler";
  version = "0.2.0";

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
  ];

  doCheck = false;

  src = ./.;
}

