package Interface.service;

import Interface.model.*;
import org.json.JSONObject;

public class JsonConverter {

    public String configToJson(SimulationConfig config) {
        JSONObject json = new JSONObject();
        json.put("topology", config.getTopology().getValue());
        json.put("routing", config.getRouting().getValue());
        json.put("width", config.getWidth());
        json.put("height", config.getHeight());
        json.put("nodes", config.getNodes());
        return json.toString();
    }

    public SimulationResult jsonToResult(String jsonStr) throws Exception {
        JSONObject json = new JSONObject(jsonStr);
        SimulationResult result = new SimulationResult();

        result.setSuccess(json.getBoolean("success"));

        if (result.isSuccess()) {
            if (json.has("latency_avg")) {
                result.setLatencyAvg(json.getDouble("latency_avg"));
            }
            if (json.has("latency_max")) {
                result.setLatencyMax(json.getDouble("latency_max"));
            }
            if (json.has("throughput")) {
                result.setThroughput(json.getDouble("throughput"));
            }
            if (json.has("packets_sent")) {
                result.setPacketsSent(json.getInt("packets_sent"));
            }
            if (json.has("packets_delivered")) {
                result.setPacketsDelivered(json.getInt("packets_delivered"));
            }
            if (json.has("packets_lost")) {
                result.setPacketsLost(json.getInt("packets_lost"));
            }
            if (json.has("cycles_elapsed")) {
                result.setCyclesElapsed(json.getLong("cycles_elapsed"));
            }
            if (json.has("execution_time_ms")) {
                result.setExecutionTimeMs(json.getLong("execution_time_ms"));
            }
        } else {
            result.setErrorMessage(json.optString("error", "Unknown error"));
        }

        return result;
    }
}
