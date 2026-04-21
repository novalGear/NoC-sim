package Interface.model;

public class SimulationConfig {
    private TopologyType topology;
    private RoutingType routing;
    private int width;
    private int height;
    private int nodes;

    public SimulationConfig() {
        this.topology = TopologyType.MESH;
        this.routing = RoutingType.OBLIVIOUS;
        this.width = 8;
        this.height = 8;
        this.nodes = 64;
    }

    public SimulationConfig(TopologyType topology, RoutingType routing,
                           int width, int height, int nodes) {
        this.topology = topology;
        this.routing = routing;
        this.width = width;
        this.height = height;
        this.nodes = nodes;
    }

    // Геттеры
    public TopologyType getTopology() { return topology; }
    public RoutingType getRouting() { return routing; }
    public int getWidth() { return width; }
    public int getHeight() { return height; }
    public int getNodes() { return nodes; }

    // Сеттеры
    public void setTopology(TopologyType topology) { this.topology = topology; }
    public void setRouting(RoutingType routing) { this.routing = routing; }
    public void setWidth(int width) { this.width = width; }
    public void setHeight(int height) { this.height = height; }
    public void setNodes(int nodes) { this.nodes = nodes; }

    public boolean isValid() {
        return width > 0 && width <= 64 &&
               height > 0 && height <= 64 &&
               nodes > 0 && nodes <= 4096 &&
               topology != null &&
               routing != null;
    }

    public String getValidationError() {
        if (width <= 0) return "Width must be positive";
        if (width > 64) return "Width cannot exceed 64";
        if (height <= 0) return "Height must be positive";
        if (height > 64) return "Height cannot exceed 64";
        if (nodes <= 0) return "Nodes must be positive";
        if (nodes > 4096) return "Nodes cannot exceed 4096";
        if (topology == null) return "Topology must be selected";
        if (routing == null) return "Routing must be selected";
        return null;
    }
}
