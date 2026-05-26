#!/bin/bash
# Usage: ./scripts/get_by_id.sh <uuid>
curl --noproxy "localhost" http://localhost:8080/api/v1/records/$1
