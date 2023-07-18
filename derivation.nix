{ lib, python3Packages }:
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

asyncjson = buildPythonPackage rec {
  pname = "asyncjson";
  version = "0.0.1";
  src = python.pkgs.fetchPypi {
    inherit pname version;
    sha256 = "sha256-CXdy000MQ+bzBzOo4Ukd+bZktFtZt5CenZK75xcPLP8=";
  };
  doCheck = false;
  propagatedBuildInputs = with pkgs; [ ];
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

in

buildPythonApplication {
  pname = "lapki-compiler";
  version = "0.1";

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
    asyncjson
    clang
  ];

  doCheck = false;

  src = ./.;
}

