#!/bin/bash
curl --noproxy "localhost" -X DELETE http://localhost:8080/api/v1/records/$1
