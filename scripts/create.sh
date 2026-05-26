#!/bin/bash
# Usage: ./scripts/create.sh <id>
ID=${1:-"test-$(date +%s)"}
curl --noproxy "localhost" -X POST http://localhost:8080/api/v1/records \
  -H "Content-Type: application/json" \
  -d "{\"id\":\"$ID\",\"block_size\":10,\"fblock\":\"test\",\"streams\":[]}"
