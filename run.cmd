@echo off
docker-compose up --build -d
docker attach sdcc_container
docker-compose down