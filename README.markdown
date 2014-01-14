# analyse

A C program to measure the difference between two JPEG images. Also measures the brightness of the first image. A component of my Raspberry Pi webcam.

## Prerequisites

Requires libjpeg. Install libjpeg on Raspberry Pi (raspbian) as follows:
    
	apt-get install libjpeg62
    apt-get install libjpeg62-dev

## Build

Build on the Raspberry Pi using `./build.sh`.

## Use

Pass the current and previous webcam images:
    
	./analyse current.jpg previous.jpg

Return format is `brightness difference` e.g. `245354 1344`. The numbers are always non-negative integers, no higher than 1000000 (one million).


