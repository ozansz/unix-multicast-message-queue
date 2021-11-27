# Multicast Shared Message Queue

Due to a subtle design fail I've made in the initial design phase, I have designed the system to **allocate 200+ shared memory segments**, which means **more than 6.5G of shared mem** on Linux systems (32M for each segment). As the implementation was already %80 done, it would need 10+ more hours to refactor or redesign the whole system. And as I didn't have much time, I am leaving the codes here unfinished.

The `RWLock`, `MessageChain` and `MessageQueue` shared data structures are working fine (tested them with 3 producers and 3-5 consumers). But the system fails to allocate memory for more clients, sadly.