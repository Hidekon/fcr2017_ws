from matplotlib import pyplot as plt
import matplotlib as mpl

class Grid2D:
    ''' Class designed to host and plot GridMap images '''

    def __init__(self):
        ''' Contructor to set default plot parameters '''
        self.cmap = mpl.colors.ListedColormap(['blue','black','red'])
        self.bounds=[-1.5,-0.5,0.5,1.5]
        self.norm = mpl.colors.BoundaryNorm(self.bounds, self.cmap.N)
        self.first_imshow = True

    def update_image(self, grid, node_name):
        ''' Perform image update on matplotlib structure '''
        # tell imshow about color map so that only set colors are used
        self.img = plt.imshow(grid,interpolation='nearest', origin='lower',
                              cmap = self.cmap, norm = self.norm)
        # iteractive mode: on
        plt.ion()
        # add plot title
        plt.title('Node: ' + node_name)

    def create_colorbar(self):
        # make a color bar
        self.cbar = plt.colorbar(self.img,
                                 cmap=self.cmap,
                                 norm=self.norm,
                                 boundaries=self.bounds,
                                 ticks=[-1,0,1])
        self.cbar.ax.set_yticklabels(['free', 'unknown', 'obstacle'])

    def show_grid2d(self, grid, node_name):
        ''' Imshow a 2D grid based on grid np.matrix input: grid '''
        # Update stored image
        self.update_image(grid, node_name)

        # first run of imshow
        if self.first_imshow:
            self.create_colorbar()
            self.first_imshow = False

        # show plot
        plt.show()
        plt.pause(0.01)

    def save_grid2d(self, grid, node_name, path):
        if not self.first_imshow:
            self.update_image(grid, node_name)
            plt.savefig(path + node_name + '.png', bbox_inches='tight')
