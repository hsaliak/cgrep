#!/bin/bash
set -e

# Colors
GREEN='\033[0;32m'
NC='\033[0m'

echo -e "${GREEN}==> Building with ASan...${NC}"
mkdir -p build
cd build
cmake .. -DENABLE_ASAN=ON -DENABLE_TSAN=OFF
make -j$(nproc 2>/dev/null || echo 4)

echo -e "\n${GREEN}==> Running Unit Tests...${NC}"
./unit_tests

echo -e "\n${GREEN}==> Running Integration Tests...${NC}"
cd ..
python3 tests/integration/test_cli.py

echo -e "\n${GREEN}==> Building with TSan...${NC}"
cd build
cmake .. -DENABLE_ASAN=OFF -DENABLE_TSAN=ON
make -j$(nproc 2>/dev/null || echo 4)

echo -e "\n${GREEN}==> Running Unit Tests (TSan)...${NC}"
./unit_tests

echo -e "\n${GREEN}==> Running Integration Tests (TSan)...${NC}"
cd ..
python3 tests/integration/test_cli.py

echo -e "\n${GREEN}Verification Complete!${NC}"
