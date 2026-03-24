#!/bin/bash
gcc sonar-radar.c -o sonar-radar -lm
sudo cp sonar-radar /usr/local/bin/sonar-radar
echo "(Fictional) sonar radar installed! Run sonar-radar anywhere in your terminal to run it."
