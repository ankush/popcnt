#!/bin/env bash

head -c ${1:-512MB} /dev/urandom > random.dat
