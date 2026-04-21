package Interface.gui;

import Interface.model.*;

import javax.swing.*;
import java.awt.*;

public class SettingsPanel extends JPanel {
    private final ButtonGroup topologyGroup;
    private final JRadioButton meshRadio;
    private final JRadioButton butterflyRadio;

    private final ButtonGroup routingGroup;
    private final JRadioButton obliviousRadio;
    private final JRadioButton adaptiveRadio;

    private final JTextField widthField;
    private final JTextField heightField;
    private final JTextField nodesField;

    public SettingsPanel() {
        setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
        setBorder(BorderFactory.createTitledBorder("Simulation Settings"));

        // Topology
        JPanel topologyPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        topologyPanel.add(new JLabel("Topology:"));
        meshRadio = new JRadioButton("MESH", true);
        butterflyRadio = new JRadioButton("BUTTERFLY");
        topologyGroup = new ButtonGroup();
        topologyGroup.add(meshRadio);
        topologyGroup.add(butterflyRadio);
        topologyPanel.add(meshRadio);
        topologyPanel.add(butterflyRadio);

        // Routing
        JPanel routingPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        routingPanel.add(new JLabel("Routing:"));
        obliviousRadio = new JRadioButton("OBLIVIOUS", true);
        adaptiveRadio = new JRadioButton("ADAPTIVE");
        routingGroup = new ButtonGroup();
        routingGroup.add(obliviousRadio);
        routingGroup.add(adaptiveRadio);
        routingPanel.add(obliviousRadio);
        routingPanel.add(adaptiveRadio);

        // Dimensions
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

        add(topologyPanel);
        add(routingPanel);
        add(dimensionsPanel);
    }

    public SimulationConfig getConfiguration() {
        SimulationConfig config = new SimulationConfig();

        config.setTopology(meshRadio.isSelected() ? TopologyType.MESH : TopologyType.BUTTERFLY);
        config.setRouting(obliviousRadio.isSelected() ? RoutingType.OBLIVIOUS : RoutingType.ADAPTIVE);

        try {
            config.setWidth(Integer.parseInt(widthField.getText().trim()));
            config.setHeight(Integer.parseInt(heightField.getText().trim()));
            config.setNodes(Integer.parseInt(nodesField.getText().trim()));
        } catch (NumberFormatException e) {
            // Возвращаем конфиг с невалидными значениями
            // Валидация будет проверена позже
        }

        return config;
    }

    public void setConfiguration(SimulationConfig config) {
        if (config.getTopology() == TopologyType.MESH) {
            meshRadio.setSelected(true);
        } else {
            butterflyRadio.setSelected(true);
        }

        if (config.getRouting() == RoutingType.OBLIVIOUS) {
            obliviousRadio.setSelected(true);
        } else {
            adaptiveRadio.setSelected(true);
        }

        widthField.setText(String.valueOf(config.getWidth()));
        heightField.setText(String.valueOf(config.getHeight()));
        nodesField.setText(String.valueOf(config.getNodes()));
    }
}
