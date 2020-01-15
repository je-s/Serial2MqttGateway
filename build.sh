#!/bin/bash

main()
{
    (
        cd ./
        make
        make clean
    )
}

main "$@"