#!/usr/bin/env bash
mv submit.zip submit-`date +%s`.zip
cd cppTracesL
zip ../submit.zip *.nbt
