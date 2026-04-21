package Interface.gui;

import Interface.model.*;

import javax.swing.*;
import java.awt.*;

public class SettingsPanel extends JPanel {
    // Topology
    private final ButtonGroup topologyGroup;
    private final JRadioButton meshRadio;
    private final JRadioButton butterflyRadio;

    // Routing
    private final ButtonGroup routingGroup;
    private final JRadioButton staticRadio;
    private final JRadioButton obliviousRadio;
    private final JRadioButton adaptiveRadio;

    // Traffic Pattern
    private final ButtonGroup trafficGroup;
    private final JRadioButton uniformRadio;
    private final JRadioButton hotspotRadio;
    private final JRadioButton syntheticRadio;

    // Dimensions
    private final JTextField widthField;
    private final JTextField heightField;
    private final JTextField nodesField;

    // Simulation parameters
    private final JTextField maxTicksField;
    private final JTextField totalPacketsField;
    private final JTextField injectionRateField;

    public SettingsPanel() {
        setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
        setBorder(BorderFactory.createTitledBorder("Simulation Settings"));

        // ===== Topology =====
        JPanel topologyPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        topologyPanel.add(new JLabel("Topology:"));
        meshRadio = new JRadioButton("MESH", true);
        butterflyRadio = new JRadioButton("BUTTERFLY");
        butterflyRadio.setEnabled(false);

        topologyGroup = new ButtonGroup();
        topologyGroup.add(meshRadio);
        topologyGroup.add(butterflyRadio);
        topologyPanel.add(meshRadio);
        topologyPanel.add(butterflyRadio);
        topologyPanel.add(new JLabel(" (only MESH supported)"));

        // ===== Routing =====
        JPanel routingPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        routingPanel.add(new JLabel("Routing:"));
        staticRadio = new JRadioButton("STATIC (X-Y)", true);
        obliviousRadio = new JRadioButton("OBLIVIOUS");
        adaptiveRadio = new JRadioButton("ADAPTIVE");
        obliviousRadio.setEnabled(false);
        adaptiveRadio.setEnabled(false);

        routingGroup = new ButtonGroup();
        routingGroup.add(staticRadio);
        routingGroup.add(obliviousRadio);
        routingGroup.add(adaptiveRadio);
        routingPanel.add(staticRadio);
        routingPanel.add(obliviousRadio);
        routingPanel.add(adaptiveRadio);
        routingPanel.add(new JLabel(" (only STATIC supported)"));

        // ===== Traffic Pattern =====
        JPanel trafficPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        trafficPanel.add(new JLabel("Traffic:"));
        uniformRadio = new JRadioButton("UNIFORM", true);
        hotspotRadio = new JRadioButton("HOTSPOT");
        syntheticRadio = new JRadioButton("SYNTHETIC");
        syntheticRadio.setEnabled(false);

        trafficGroup = new ButtonGroup();
        trafficGroup.add(uniformRadio);
        trafficGroup.add(hotspotRadio);
        trafficGroup.add(syntheticRadio);
        trafficPanel.add(uniformRadio);
        trafficPanel.add(hotspotRadio);
        trafficPanel.add(syntheticRadio);

        // ===== Dimensions =====
        JPanel dimensionsPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        widthField = new JTextField("8", 5);
        heightField = new JTextField("8", 5);
        nodesField = new JTextField("64", 5);

        dimensionsPanel.add(new JLabel("Width:"));
        dimensionsPanel.add(widthField);
        dimensionsPanel.add(Box.createHorizontalStrut(10));
        dimensionsPanel.add(new JLabel("Height:"));
        dimensionsPanel.add(heightField);
        dimensionsPanel.add(Box.createHorizontalStrut(10));
        dimensionsPanel.add(new JLabel("Nodes:"));
        dimensionsPanel.add(nodesField);

        // ===== Max Ticks =====
        JPanel maxTicksPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        maxTicksField = new JTextField("10000", 8);

        maxTicksPanel.add(new JLabel("Max Ticks:"));
        maxTicksPanel.add(maxTicksField);
        maxTicksPanel.add(new JLabel(" (1 - 1,000,000)"));

        // ===== Traffic Parameters =====
        JPanel trafficParamsPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        totalPacketsField = new JTextField("1000", 8);
        injectionRateField = new JTextField("0.1", 5);

        trafficParamsPanel.add(new JLabel("Total Packets:"));
        trafficParamsPanel.add(totalPacketsField);
        trafficParamsPanel.add(Box.createHorizontalStrut(15));
        trafficParamsPanel.add(new JLabel("Injection Rate:"));
        trafficParamsPanel.add(injectionRateField);
        trafficParamsPanel.add(new JLabel(" (0.0 - 1.0)"));

        // ===== Add all panels =====
        add(topologyPanel);
        add(routingPanel);
        add(trafficPanel);
        add(dimensionsPanel);
        add(maxTicksPanel);
        add(trafficParamsPanel);

        // ===== Info label =====
        JLabel infoLabel = new JLabel("Note: Only MESH topology with STATIC (X-Y) routing is currently implemented");
        infoLabel.setFont(new Font("Arial", Font.ITALIC, 11));
        infoLabel.setForeground(Color.GRAY);
        infoLabel.setBorder(BorderFactory.createEmptyBorder(5, 10, 5, 10));
        add(infoLabel);
    }

    public SimulationConfig getConfiguration() {
        SimulationConfig config = new SimulationConfig();

        // Topology
        config.setTopology(meshRadio.isSelected() ? TopologyType.MESH : TopologyType.BUTTERFLY);

        // Routing
        if (staticRadio.isSelected()) {
            config.setRouting(RoutingType.STATIC);
        } else if (obliviousRadio.isSelected()) {
            config.setRouting(RoutingType.OBLIVIOUS);
        } else {
            config.setRouting(RoutingType.ADAPTIVE);
        }

        // Traffic pattern
        if (uniformRadio.isSelected()) {
            config.setTraffic(TrafficPatternType.UNIFORM);
        } else if (hotspotRadio.isSelected()) {
            config.setTraffic(TrafficPatternType.HOTSPOT);
        } else {
            config.setTraffic(TrafficPatternType.SYNTHETIC);
        }

        // Dimensions
        try {
            config.setWidth(Integer.parseInt(widthField.getText().trim()));
            config.setHeight(Integer.parseInt(heightField.getText().trim()));
            config.setNodes(Integer.parseInt(nodesField.getText().trim()));
        } catch (NumberFormatException e) {
            // Оставляем невалидные значения для последующей проверки
        }

        // Max ticks
        try {
            config.setMaxTicks(Integer.parseInt(maxTicksField.getText().trim()));
        } catch (NumberFormatException e) {
            config.setMaxTicks(-1);
        }

        // Total packets
        try {
            config.setTotalPackets(Integer.parseInt(totalPacketsField.getText().trim()));
        } catch (NumberFormatException e) {
            config.setTotalPackets(-1);
        }

        // Injection rate
        try {
            config.setInjectionRate(Double.parseDouble(injectionRateField.getText().trim()));
        } catch (NumberFormatException e) {
            config.setInjectionRate(-1.0);
        }

        return config;
    }

    public void setConfiguration(SimulationConfig config) {
        // Topology
        if (config.getTopology() == TopologyType.MESH) {
            meshRadio.setSelected(true);
        } else {
            butterflyRadio.setSelected(true);
        }

        // Routing
        switch (config.getRouting()) {
            case STATIC:
                staticRadio.setSelected(true);
                break;
            case OBLIVIOUS:
                obliviousRadio.setSelected(true);
                break;
            case ADAPTIVE:
                adaptiveRadio.setSelected(true);
                break;
        }

        // Traffic
        switch (config.getTraffic()) {
            case UNIFORM:
                uniformRadio.setSelected(true);
                break;
            case HOTSPOT:
                hotspotRadio.setSelected(true);
                break;
            case SYNTHETIC:
                syntheticRadio.setSelected(true);
                break;
        }

        // Dimensions
        widthField.setText(String.valueOf(config.getWidth()));
        heightField.setText(String.valueOf(config.getHeight()));
        nodesField.setText(String.valueOf(config.getNodes()));

        // Max ticks
        maxTicksField.setText(String.valueOf(config.getMaxTicks()));

        // Traffic parameters
        totalPacketsField.setText(String.valueOf(config.getTotalPackets()));
        injectionRateField.setText(String.valueOf(config.getInjectionRate()));
    }
}
