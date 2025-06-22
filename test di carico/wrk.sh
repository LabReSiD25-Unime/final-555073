#!/bin/bash

for i in {1..20}; do
    wrk -t4 -c100 -d30s http://localhost:8080/ >> wrk_get.text
    echo {$i}
done

for i in {1..20}; do
    wrk -t4 -c1000 -d30s http://localhost:8080/ >> wrk_get.text
    echo {$i}
done

for i in {1..20}; do
    wrk -t4 -c10000 -d30s http://localhost:8080/ >> wrk_get.text
    echo {$i}
done
