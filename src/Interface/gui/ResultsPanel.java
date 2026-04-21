package Interface.gui;

import Interface.model.SimulationResult;

import javax.swing.*;
import javax.swing.table.DefaultTableModel;
import java.awt.*;

public class ResultsPanel extends JPanel {
    private final JTable statsTable;
    private final DefaultTableModel tableModel;
    private final JTextArea detailsArea;

    public ResultsPanel() {
        setLayout(new BorderLayout());
        setBorder(BorderFactory.createTitledBorder("Results"));

        // Таблица со статистикой
        String[] columns = {"Metric", "Value"};
        tableModel = new DefaultTableModel(columns, 0);
        statsTable = new JTable(tableModel);
        statsTable.setFont(new Font("Arial", Font.PLAIN, 12));
        statsTable.getColumnModel().getColumn(0).setPreferredWidth(180);
        statsTable.getColumnModel().getColumn(1).setPreferredWidth(150);
        statsTable.setRowHeight(25);

        JScrollPane tableScroll = new JScrollPane(statsTable);
        tableScroll.setPreferredSize(new Dimension(350, 200));

        // Детальная информация
        detailsArea = new JTextArea();
        detailsArea.setEditable(false);
        detailsArea.setFont(new Font("Consolas", Font.PLAIN, 12));
        detailsArea.setBackground(new Color(245, 245, 245));

        JSplitPane splitPane = new JSplitPane(
            JSplitPane.VERTICAL_SPLIT,
            tableScroll,
            new JScrollPane(detailsArea)
        );
        splitPane.setResizeWeight(0.6);

        add(splitPane, BorderLayout.CENTER);

        // Начальное состояние
        clear();
    }

    public void updateResults(SimulationResult result) {
        tableModel.setRowCount(0);
        detailsArea.setText("");

        tableModel.addRow(new Object[]{"Status", result.isSuccess() ? "✓ SUCCESS" : "✗ FAILED"});

        if (result.isSuccess()) {
            if (result.getLatencyAvg() > 0) {
                tableModel.addRow(new Object[]{"Average Latency",
                    String.format("%.3f cycles", result.getLatencyAvg())});
            }
            if (result.getLatencyMax() > 0) {
                tableModel.addRow(new Object[]{"Maximum Latency",
                    String.format("%.3f cycles", result.getLatencyMax())});
            }
            if (result.getThroughput() > 0) {
                tableModel.addRow(new Object[]{"Throughput",
                    String.format("%.3f pkts/cycle", result.getThroughput())});
            }
            if (result.getAverageHops() > 0) {
                tableModel.addRow(new Object[]{"Average Hops",
                    String.format("%.2f", result.getAverageHops())});
            }
            if (result.getTotalPackets() > 0) {
                tableModel.addRow(new Object[]{"Total Packets", result.getTotalPackets()});
            }
            if (result.getPacketsSent() > 0) {
                tableModel.addRow(new Object[]{"Packets Sent", result.getPacketsSent()});
            }
            if (result.getPacketsDelivered() > 0) {
                tableModel.addRow(new Object[]{"Packets Delivered", result.getPacketsDelivered()});
            }
            if (result.getPacketsLost() >= 0) {
                tableModel.addRow(new Object[]{"Packets Lost", result.getPacketsLost()});
            }
            if (result.getDeliveryRate() > 0) {
                tableModel.addRow(new Object[]{"Delivery Rate",
                    String.format("%.2f%%", result.getDeliveryRate() * 100)});
            }
            if (result.getPacketLossRate() > 0) {
                tableModel.addRow(new Object[]{"Packet Loss Rate",
                    String.format("%.2f%%", result.getPacketLossRate())});
            }
            if (result.getCyclesElapsed() > 0) {
                tableModel.addRow(new Object[]{"Cycles Elapsed", result.getCyclesElapsed()});
            }
            if (result.getExecutionTimeMs() > 0) {
                tableModel.addRow(new Object[]{"Execution Time",
                    result.getExecutionTimeMs() + " ms"});
            }

            // Детальное описание
            StringBuilder details = new StringBuilder();
            details.append("SIMULATION DETAILS\n");
            details.append("==================\n\n");
            details.append(String.format("Topology: MESH %dx%d\n",
                (int)Math.sqrt(result.getPacketsSent() / 10),
                (int)Math.sqrt(result.getPacketsSent() / 10)));
            details.append(String.format("Total packets: %d\n", result.getTotalPackets()));
            details.append(String.format("Successfully delivered: %d (%.1f%%)\n",
                result.getPacketsDelivered(), result.getDeliveryRate() * 100));
            details.append(String.format("Lost in transit: %d (%.1f%%)\n",
                result.getPacketsLost(), result.getPacketLossRate()));
            details.append(String.format("\nAverage latency: %.3f cycles\n", result.getLatencyAvg()));
            details.append(String.format("Average hops: %.2f\n", result.getAverageHops()));
            details.append(String.format("Throughput: %.3f packets/cycle\n", result.getThroughput()));
            details.append(String.format("\nExecution time: %d ms\n", result.getExecutionTimeMs()));

            detailsArea.setText(details.toString());
        } else {
            tableModel.addRow(new Object[]{"Error", result.getErrorMessage()});
            detailsArea.setText("Simulation failed:\n" + result.getErrorMessage());
        }
    }

    public void clear() {
        tableModel.setRowCount(0);
        tableModel.addRow(new Object[]{"Status", "No simulation run yet"});
        detailsArea.setText("Run a simulation to see results here.");
    }
}
