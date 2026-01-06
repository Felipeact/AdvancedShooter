# Advanced Shooter (Unreal Engine 5)

## Overview
Advanced Shooter is a systems-focused FPS project developed in **Unreal Engine 5 using C++**.  
The project was built to demonstrate **scalable gameplay systems**, clean architecture, and performance-aware design rather than a single gameplay feature.

This project emphasizes **reusable systems**, multiplayer-ready logic, and a clear separation between core systems and game-specific behavior.

---

## Core Gameplay Systems

### 🔹 Weapon System
- Modular weapon architecture
- Supports hitscan and projectile-based weapons
- Data-driven weapon configuration
- Extendable fire modes

**Technical Notes**
- Weapon logic is decoupled from character code
- No per-frame ticking; event-driven firing logic

---

### 🔹 Health & Damage System
- Centralized damage handling
- Supports different damage types
- Clean API for AI and player interaction

---

### 🔹 AI Systems
- AI perception-based behavior
- Behavior Tree–driven decision making
- Configurable AI parameters via data assets

**Performance Considerations**
- Reduced tick usage
- Controlled perception update frequency

---

### 🔹 Player Controller & Character Architecture
- Clean separation between input, movement, and gameplay logic
- C++ core with Blueprint exposure for designers

---

## Architecture Highlights
- Strong separation of concerns
- Component-based design
- Minimal Blueprint logic (Blueprints are designer-facing only)
- Clear ownership and responsibility boundaries

---

## Performance & Optimization
- Reduced unnecessary ticking
- Explicit control over update frequency
- Structured logging for debugging gameplay flow

---

## Tech Stack
- Unreal Engine 5
- C++
- Unreal Gameplay Framework
- Behavior Trees
- AI Perception System

---

## Project Goals
- Demonstrate gameplay systems architecture
- Show clean Unreal Engine C++ practices
- Serve as a foundation for expanding into multiplayer systems

---

## Author
**Felipe Augusto**  
Gameplay / Gameplay Systems Programmer  
Focus: Unreal Engine, C++, Gameplay Architecture

