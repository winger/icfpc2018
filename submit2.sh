#!/usr/bin/env bash

set -e

SHA=`shasum -a 256 submit.zip | awk '{print $1}'`
ID=`cat private.id`
curl -L \
    --data-urlencode action=submit \
    --data-urlencode privateID=${ID} \
    --data-urlencode submissionURL=$1 \
    --data-urlencode submissionSHA=${SHA} \
    https://script.google.com/macros/s/AKfycbzQ7Etsj7NXCN5thGthCvApancl5vni5SFsb1UoKgZQwTzXlrH7/exec
