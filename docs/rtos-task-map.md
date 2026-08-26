# RTOS Task Map

## Planned Tasks

### input_task
**Purpose:**
- Monitor manual hardware controls
- Handle microphone mute and cloud-enable state changes
- Notify affected system services

**Activation:**
- GPIO event / task notification or periodic polling

**Priority:**
- High when active

### audio_feed_task
**Purpose:**
- Capture PCM audio from the INMP441 through I2S
- Supply audio frames to the voice-processing pipeline

**Activation:**
- Continuous while microphone processing is enabled

**Priority:**
- High

### voice_ai_task
**Purpose:**
- Run local wake-word and command recognition
- Generate recognized command/intent events

**Activation:**
- Audio input / wake event

**Priority:**
- High

### command_router_task
**Purpose:**
- Determine whether a command is handled locally, forwarded to the Raspberry Pi,
forwarded to cloud services, or rejected

**Activation:**
- Recognized intent/message queue

**Priority:**
- Medium/High

### network_task
**Purpose:**
- Manage Wi-Fi and/or higher-level communicaiton
- Support Pi and cloud/API traffic

**Priority:**
- Medium

### sensor_task
**Purpose:**
- Periodically acquire BME280 measurements

**Priority:**
- Low/Medium

### lighting_task
**Purpose:**
- Control WS2812B output and system-status lighting

**Priority:**
- Low/Medium

### health_task
**Purpose:**
- Monitor system state, errors, memory, and diagnostics

**Priority:**
- Low

## ISR Design Rule
Interrupt service routines will perform only minimal, time-critical work.

**Possible Inclusions:**
- Clear interrupt conditions
- Capture small amounts of state
- Notify/unblock FreeRTOS tasks

**Possible Exclusions:**
- Perform AI inference
- Execute network operations
- Perform lengthy sensor transactions
- Update complex LED effects
- Perform lengthy logging
