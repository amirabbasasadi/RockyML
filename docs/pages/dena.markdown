# Dena{#dena}

Let's see a simple example of an optimizer written in Dena:

```cpp
auto optimizer = container::create("A", n_particles, group_size)
                 >> pso::memory::create("M", "A")
                 >> init::uniform("A")
                 >> run::n_times(20, pso::local::step("M", "A"));
```

After designing the optimizer we can execute it using Zagros runtime:
```cpp
zagros::basic_runtime<swarm_type, dim, block_dim> runtime(&problem);
runtime.run(f1);
```