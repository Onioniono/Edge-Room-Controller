# System Architecture

## ESP32-S3
Primary Embedded Controller and hardware authority
Responsibilities:
- Local voice-command recognition
- Audio acquisition
- Sensor acquisitio
- Physical device control
- Manual privacy controls
- Local automation
- Local/Pi/cloud decision routing
- Wi-Fi communication

Note: The ESP32-S3 shall retain local operation even if the 
Raspberry Pi or cloud connection is unavailable.

## Raspberry Pi 5
Higher-level Linux edge-compute and HMI node
Planned Responsibilities:
- Local speech-to-text
- Flexible intent interpretation
- Cloud/API integration
- Mirror user interface
- Logging and higher-level data processing

## Fundamental Principle
The Raspberry Pi may interpret or recommend actions; however, the ESP32-S3
remains responsible for executing physical control operations.
