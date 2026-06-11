#!/bin/bash
curl --noproxy "localhost" -s http://localhost:8080/stats | python3 -m json.tool
