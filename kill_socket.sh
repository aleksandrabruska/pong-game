#!/bin/bash
kill -9 $(fuser 8080/tcp 2>/dev/null)
