#!/bin/bash

# Copy template as arg1 folder
cp -r template/ $1

# Find and replace 'template' with arg1 in all files
find ./$1 -type f -exec sed -i "s/template/$1/g" {} \;

# Rename template.c to arg1.c
cd $1 && mv template.c $1.c && cd ..