#!/usr/bin/env bash
mv submit.zip submit-`date +%s`.zip
cd submitTracesF
zip ../submit-`date +%s`.zip *.nbt
