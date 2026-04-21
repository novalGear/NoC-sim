package Interface.model;

public class SimulationResult {
    private boolean success;
    private double latencyAvg;
    private double latencyMax;
    private double throughput;
    private int packetsSent;
    private int packetsDelivered;
    private int packetsLost;
    private long cyclesElapsed;
    private long executionTimeMs;
    private String errorMessage;

    private double averageHops;
    private double deliveryRate;
    private int totalPackets;

    public SimulationResult() {
        this.success = false;
        this.errorMessage = "";
        this.averageHops = 0.0;
        this.deliveryRate = 0.0;
        this.totalPackets = 0;
    }

    // Геттеры
    public boolean isSuccess() { return success; }
    public double getLatencyAvg() { return latencyAvg; }
    public double getLatencyMax() { return latencyMax; }
    public double getThroughput() { return throughput; }
    public int getPacketsSent() { return packetsSent; }
    public int getPacketsDelivered() { return packetsDelivered; }
    public int getPacketsLost() { return packetsLost; }
    public long getCyclesElapsed() { return cyclesElapsed; }
    public long getExecutionTimeMs() { return executionTimeMs; }
    public String getErrorMessage() { return errorMessage; }
    public double getAverageHops() { return averageHops; }
    public double getDeliveryRate() { return deliveryRate; }
    public int getTotalPackets() { return totalPackets; }

    // Сеттеры
    public void setSuccess(boolean success) { this.success = success; }
    public void setLatencyAvg(double latencyAvg) { this.latencyAvg = latencyAvg; }
    public void setLatencyMax(double latencyMax) { this.latencyMax = latencyMax; }
    public void setThroughput(double throughput) { this.throughput = throughput; }
    public void setPacketsSent(int packetsSent) { this.packetsSent = packetsSent; }
    public void setPacketsDelivered(int packetsDelivered) { this.packetsDelivered = packetsDelivered; }
    public void setPacketsLost(int packetsLost) { this.packetsLost = packetsLost; }
    public void setCyclesElapsed(long cyclesElapsed) { this.cyclesElapsed = cyclesElapsed; }
    public void setExecutionTimeMs(long executionTimeMs) { this.executionTimeMs = executionTimeMs; }
    public void setErrorMessage(String errorMessage) { this.errorMessage = errorMessage; }
    public void setAverageHops(double averageHops) { this.averageHops = averageHops; }
    public void setDeliveryRate(double deliveryRate) { this.deliveryRate = deliveryRate; }
    public void setTotalPackets(int totalPackets) { this.totalPackets = totalPackets; }

    public double getPacketLossRate() {
        if (packetsSent == 0) return 0.0;
        return (double) packetsLost / packetsSent * 100.0;
    }
}
