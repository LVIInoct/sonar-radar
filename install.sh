#!/bin/bash
gcc sonarradar.c -o sonarradar -lm
sudo cp sonarradar /usr/local/bin/sonarradar
echo "(Fictional) sonar radar installed."
