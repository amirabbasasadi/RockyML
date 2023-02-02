# Etna | Non-differentiable Neural Networks{#etna}

## Design Goals
The main gloal of Etna is to provide fast and thread-safe implementations of non-different deep learning components including:
- Integer-valued layers
- Combinatorial components
- Discrete activation functions
- Stochastic components

Non-differentiable deep learning components are not so famous since such models can not be optimized using gradient-based algorithms like SGD. But don't worry! Etna components can be optimized using powerful gradient-free optimizers available in Zagros.  