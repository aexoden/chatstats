#!/bin/bash

DEFAULT_VERSION=v0.0.2

if [ -d ../.git ] ; then
	VERSION=$(git describe --dirty 2>/dev/null)
else
	VERSION=$DEFAULT_VERSION
fi

echo "#define CHATSTATS_VERSION \"$VERSION\""
