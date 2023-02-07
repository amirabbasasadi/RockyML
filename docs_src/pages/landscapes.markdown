# Landscapes, Test problems for optimization{#landscapes}
 <table>
  <tr>
    <th>Function</th>
    <th>Definition</th>
    <th>Heat Map</th>
    <th>Surface</th>
  </tr>
  <tr>
    <td>@link rocky::zagros::benchmark::ackley Ackley Function</td>
    <td>\f$ f(\mathbf {x} )= -20 \exp\left[-0.2\sqrt{ \frac{1}{n}\sum _{i=1}^n{ x_i^2 } }  \right] \\ - \exp\left[ \frac{1}{n}\sum _{i=1}^n{ cos(2 \pi x_i) }  \right] + 20 + e \f$<br>
    <br>Global minimum at \f$ \mathbf{x} = 0 \f$</td>
    <td><img src="zagros_ackley_0_heatmap.png" width="250px"></td>
    <td><img src="zagros_ackley_0_surface.png" width="250px"></td>
  </tr>
  <tr>
    <td>@link rocky::zagros::benchmark::rastrigin Rastrigin Function</td>
    <td>\f$ f(\mathbf {x} )=10n+\sum _{i=1}^{n}\left[x_{i}^{2}-10\cos(2\pi x_{i})\right] \f$<br>
    <br>Global minimum at \f$ \mathbf{x} = 0 \f$</td>
    <td><img src="zagros_rastrigin_0_heatmap.png" width="250px"></td>
    <td><img src="zagros_rastrigin_0_surface.png" width="250px"></td>
  </tr>
  <tr>
    <td>@link rocky::zagros::benchmark::dropwave Dropwave Function</td>
    <td>\f$ f(\mathbf {x} )= -\frac{1 + \cos(12\sqrt{\sum_{i=1}^{n}x_{i}^{2})}}{0.5\left[\sum_{i=1}^{n}x_{i}^{2}\right] + 2} \f$<br>
    <br>Global minimum at \f$ \mathbf{x} = 0 \f$</td>
    <td><img src="zagros_dropwave_0_heatmap.png" width="250px"></td>
    <td><img src="zagros_dropwave_0_surface.png" width="250px"></td>
  </tr>
  <tr>
    <td>@link rocky::zagros::benchmark::griewank Griewank Function</td>
    <td>\f$ f(\mathbf {x} )=1 + \sum _{i=1}^{n}\frac{x_{i}^{2}}{4000} - \prod _{i=1}^{n}\cos{\frac{x_{i}}{\sqrt{i}}} \f$<br>
    <br>Global minimum at \f$ \mathbf{x} = 0 \f$</td>
    <td><img src="zagros_griewank_0_heatmap.png" width="250px"></td>
    <td><img src="zagros_griewank_0_surface.png" width="250px"></td>
  </tr>
  <tr>
    <td>@link rocky::zagros::benchmark::sphere Sphere Function</td>
    <td>\f$ f(\mathbf {x} )=\sum _{i=1}^{n}x_{i}^{2} \f$<br>
    <br>Global minimum at \f$ \mathbf{x} = 0 \f$</td>
    <td><img src="zagros_sphere_0_heatmap.png" width="250px"></td>
    <td><img src="zagros_sphere_0_surface.png" width="250px"></td>
  </tr>
  <tr>
    <td>@link rocky::zagros::benchmark::rosenbrock Rosenbrock Function</td>
    <td>\f$ f({\mathbf {x}})=\sum _{i=1}^{n-1}\left[100\left(x_{i+1}-x_{i}^{2}\right)^{2}+\left(1-x_{i}\right)^{2}\right] \f$<br>
    <br>Global minimum at \f$ \mathbf{x} = \mathbf{1} \f$</td>
    <td><img src="zagros_rosenbrock_0_heatmap.png" width="250px"></td>
    <td><img src="zagros_rosenbrock_0_surface.png" width="250px"></td>
  </tr>
</table> 

## References
- https://www.sfu.ca/~ssurjano/griewank.html
- https://www.sfu.ca/~ssurjano/drop.html