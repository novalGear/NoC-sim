package Interface.gui;

import Interface.exception.SimulationException;
import Interface.model.SimulationConfig;
import Interface.model.SimulationResult;
import Interface.service.SimulationService;

import javax.swing.*;
import java.awt.*;

public class InterconnectFrame extends JFrame {
    private final SettingsPanel settingsPanel;
    private final ResultsPanel resultsPanel;
    private final LogPanel logPanel;
    private final ControlPanel controlPanel;

    private final SimulationService simulationService;
    private SimulationWorker currentWorker;

    public InterconnectFrame() {
        super("Interconnect Simulator GUI");
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setSize(900, 800);
        setLayout(new BorderLayout());

        simulationService = new SimulationService();

        // Инициализация панелей
        settingsPanel = new SettingsPanel();
        resultsPanel = new ResultsPanel();
        logPanel = new LogPanel();
        controlPanel = new ControlPanel();

        // Добавление слушателей
        controlPanel.addRunListener(e -> runSimulation());
        controlPanel.addClearListener(e -> logPanel.clear());

        // Компоновка
        add(settingsPanel, BorderLayout.NORTH);

        JSplitPane centerSplit = new JSplitPane(
            JSplitPane.HORIZONTAL_SPLIT,
            resultsPanel,
            logPanel
        );
        centerSplit.setResizeWeight(0.5);
        add(centerSplit, BorderLayout.CENTER);

        add(controlPanel, BorderLayout.SOUTH);

        logPanel.log("Application initialized", LogPanel.LogLevel.INFO);
        logPanel.log("Native library loaded successfully", LogPanel.LogLevel.INFO);
    }

    private void runSimulation() {
        if (controlPanel.isRunning()) {
            logPanel.log("Simulation already running", LogPanel.LogLevel.WARNING);
            return;
        }

        // Получение конфигурации
        SimulationConfig config = settingsPanel.getConfiguration();

        // Валидация
        if (!config.isValid()) {
            String error = config.getValidationError();
            logPanel.log("Invalid configuration: " + error, LogPanel.LogLevel.ERROR);
            JOptionPane.showMessageDialog(this,
                "Invalid configuration:\n" + error,
                "Validation Error",
                JOptionPane.ERROR_MESSAGE);
            return;
        }

        // Запуск симуляции
        controlPanel.setRunning(true);
        logPanel.log("Starting simulation with config: " +
            config.getTopology() + ", " + config.getRouting() +
            ", " + config.getWidth() + "x" + config.getHeight(),
            LogPanel.LogLevel.INFO);

        currentWorker = new SimulationWorker(config);
        currentWorker.execute();
    }

    private class SimulationWorker extends SwingWorker<SimulationResult, String> {
        private final SimulationConfig config;

        public SimulationWorker(SimulationConfig config) {
            this.config = config;
        }

        @Override
        protected SimulationResult doInBackground() throws Exception {
            publish("Initializing simulation...");

            try {
                SimulationResult result = simulationService.runSimulation(config);
                publish("Simulation completed successfully");
                return result;
            } catch (SimulationException e) {
                publish("ERROR: " + e.getMessage());
                throw e;
            }
        }

        @Override
        protected void process(java.util.List<String> chunks) {
            for (String message : chunks) {
                if (message.startsWith("ERROR:")) {
                    logPanel.log(message, LogPanel.LogLevel.ERROR);
                } else {
                    logPanel.log(message, LogPanel.LogLevel.INFO);
                }
            }
        }

        @Override
        protected void done() {
            try {
                SimulationResult result = get();

                if (result.isSuccess()) {
                    logPanel.log("✓ Simulation succeeded", LogPanel.LogLevel.INFO);
                    logPanel.log(String.format("  Avg latency: %.3f cycles", result.getLatencyAvg()));
                    logPanel.log(String.format("  Throughput: %.2f pkts/cycle", result.getThroughput()));
                    logPanel.log(String.format("  Packets: %d delivered, %d lost",
                        result.getPacketsDelivered(), result.getPacketsLost()));

                    resultsPanel.updateResults(result);
                } else {
                    logPanel.log("✗ Simulation failed: " + result.getErrorMessage(),
                        LogPanel.LogLevel.ERROR);
                    resultsPanel.updateResults(result);

                    JOptionPane.showMessageDialog(InterconnectFrame.this,
                        "Simulation failed:\n" + result.getErrorMessage(),
                        "Simulation Error",
                        JOptionPane.ERROR_MESSAGE);
                }

            } catch (Exception e) {
                logPanel.log("Simulation error: " + e.getMessage(), LogPanel.LogLevel.ERROR);

                SimulationResult errorResult = new SimulationResult();
                errorResult.setSuccess(false);
                errorResult.setErrorMessage(e.getMessage());
                resultsPanel.updateResults(errorResult);

                JOptionPane.showMessageDialog(InterconnectFrame.this,
                    "Simulation error:\n" + e.getMessage(),
                    "Error",
                    JOptionPane.ERROR_MESSAGE);
            } finally {
                controlPanel.setRunning(false);
                currentWorker = null;
            }
        }
    }
}
