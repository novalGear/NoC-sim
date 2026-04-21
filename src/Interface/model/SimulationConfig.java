package Interface.model;

public class SimulationConfig {
    private TopologyType topology;
    private RoutingType routing;
    private TrafficPatternType traffic;
    private int width;
    private int height;
    private int nodes;
    private int maxTicks;
    private int totalPackets;
    private double injectionRate;

    public SimulationConfig() {
        this.topology = TopologyType.MESH;
        this.routing = RoutingType.STATIC;
        this.traffic = TrafficPatternType.UNIFORM;
        this.width = 8;
        this.height = 8;
        this.nodes = 64;
        this.maxTicks = 10000;
        this.totalPackets = 1000;
        this.injectionRate = 0.1;
    }

    public SimulationConfig(TopologyType topology, RoutingType routing,
                           TrafficPatternType traffic, int width, int height,
                           int nodes, int maxTicks, int totalPackets, double injectionRate) {
        this.topology = topology;
        this.routing = routing;
        this.traffic = traffic;
        this.width = width;
        this.height = height;
        this.nodes = nodes;
        this.maxTicks = maxTicks;
        this.totalPackets = totalPackets;
        this.injectionRate = injectionRate;
    }

    // Геттеры
    public TopologyType getTopology() { return topology; }
    public RoutingType getRouting() { return routing; }
    public TrafficPatternType getTraffic() { return traffic; }
    public int getWidth() { return width; }
    public int getHeight() { return height; }
    public int getNodes() { return nodes; }
    public int getMaxTicks() { return maxTicks; }
    public int getTotalPackets() { return totalPackets; }
    public double getInjectionRate() { return injectionRate; }

    // Сеттеры
    public void setTopology(TopologyType topology) { this.topology = topology; }
    public void setRouting(RoutingType routing) { this.routing = routing; }
    public void setTraffic(TrafficPatternType traffic) { this.traffic = traffic; }
    public void setWidth(int width) { this.width = width; }
    public void setHeight(int height) { this.height = height; }
    public void setNodes(int nodes) { this.nodes = nodes; }
    public void setMaxTicks(int maxTicks) { this.maxTicks = maxTicks; }
    public void setTotalPackets(int totalPackets) { this.totalPackets = totalPackets; }
    public void setInjectionRate(double injectionRate) { this.injectionRate = injectionRate; }

    public boolean isValid() {
        if (topology != TopologyType.MESH) {
            return false;
        }
        if (routing != RoutingType.STATIC) {
            return false;
        }
        if (traffic == null) {
            return false;
        }
        if (maxTicks <= 0 || maxTicks > 1000000) {
            return false;
        }
        if (totalPackets <= 0 || totalPackets > 100000) {
            return false;
        }
        if (injectionRate <= 0.0 || injectionRate > 1.0) {
            return false;
        }

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
        if (traffic == null) return "Traffic pattern must be selected";
        if (topology != TopologyType.MESH) {
            return "Only MESH topology is implemented so far";
        }
        if (routing != RoutingType.STATIC) {
            return "Only STATIC (X-Y) routing is implemented so far";
        }
        if (traffic == TrafficPatternType.SYNTHETIC) {
            return "SYNTHETIC traffic pattern is not implemented yet";
        }
        if (maxTicks <= 0) {
            return "Max ticks must be positive";
        }
        if (maxTicks > 1000000) {
            return "Too many max ticks. Enter less than 1,000,000";
        }
        if (totalPackets <= 0) {
            return "Total packets must be positive";
        }
        if (totalPackets > 100000) {
            return "Total packets cannot exceed 100,000";
        }
        if (injectionRate <= 0.0) {
            return "Injection rate must be positive";
        }
        if (injectionRate > 1.0) {
            return "Injection rate cannot exceed 1.0";
        }
        return null;
    }
}
