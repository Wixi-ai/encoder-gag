#!/bin/bash
# Статистика по БД

echo "=== RECORDS STATS ==="
sqlite3 records.db "SELECT COUNT(*) as total_records FROM records;"

echo ""
echo "=== VAA BLOCKS STATS ==="
sqlite3 records.db "SELECT COUNT(*) as total_blocks FROM vaa_blocks;"

echo ""
echo "=== BLOCKS BY TYPE ==="
sqlite3 records.db "SELECT block_type, COUNT(*) FROM vaa_blocks GROUP BY block_type;"

echo ""
echo "=== LAST 5 RECORDS ==="
sqlite3 records.db "SELECT id, created_at FROM records ORDER BY created_at DESC LIMIT 5;"
