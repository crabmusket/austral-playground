default:
  @just --list

build-all:
  just sockets/build
  just assoc/build
  just small/build
