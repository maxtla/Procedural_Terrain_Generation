## Procedural Terrain Generation with DirectX 12

**Author:** Max Tlatlik

**Supervisor:** Stefan Petersson, Hans Tap

This project is part of my Bachelor Thesis and explores the topic of procedural terrain generation. 
Specifically it aims to implement and examine an algorithm for procedural terrain generation on the GPU 
with the modern graphics API DirectX 12. The algorithm for procedural terrain generation is based on the
previous work done in the book *GPU Gems 3*, Chapter 1, by Hubert Nguyen. The goal is to implement the marching cubes algorithm 
with async compute and to examine the performance for async vs non-async. 

The framework is build using SDL 2.0 for window, event management and logging, and with DirectX 12 as the custom renderer.
ImGui is added for simple and easy UI management. 
