import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import org.json.JSONObject; // Импорт библиотеки

public class InterconnectGUI extends JFrame implements ActionListener {

    // Native методы
    private native int nativeInitialize(String jsonConfig);
    private native String nativeRun();
    private native String nativeGetLastError();

    static {
        System.loadLibrary("JNIdll");
    }

    // === ПЕРЕМЕННЫЕ ИНТЕРФЕЙСА ===
    private ButtonGroup buttonGroupTopology;
    private JRadioButton radioButtonMesh;
    private JRadioButton radioButtonButterfly;
    private ButtonGroup buttonGroupRouting;
    private JRadioButton radioButtonOblivious;
    private JRadioButton radioButtonAdaptive;
    private JTextField textFieldWidth;
    private JTextField textFieldHeight;
    private JTextField textFieldNodes;
    private JTextArea textAreaLog;
    private JButton buttonRun;

    public InterconnectGUI() {
        super("Interconnect Simulator GUI (JSON Edition)");
        this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        this.setSize(800, 700);
        this.setLayout(new BorderLayout());

        initPanelSettings();
        initPanelLog();
        initPanelControls();

        this.setVisible(true);
        log("System initialized. JSON libraries ready.");
    }

    private void initPanelSettings() {
        JPanel panelSettings = new JPanel(new GridLayout(3, 1));
        panelSettings.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));

        // Topology
        JLabel labelTopology = new JLabel("Topology:");
        radioButtonMesh = new JRadioButton("MESH", true);
        radioButtonButterfly = new JRadioButton("BUTTERFLY", false);
        buttonGroupTopology = new ButtonGroup();
        buttonGroupTopology.add(radioButtonMesh);
        buttonGroupTopology.add(radioButtonButterfly);
        JPanel panelTopology = new JPanel(new FlowLayout(FlowLayout.LEFT));
        panelTopology.add(labelTopology);
        panelTopology.add(radioButtonMesh);
        panelTopology.add(radioButtonButterfly);
        panelSettings.add(panelTopology);

        // Routing
        JLabel labelRouting = new JLabel("Routing:");
        radioButtonOblivious = new JRadioButton("OBLIVIOUS", true);
        radioButtonAdaptive = new JRadioButton("ADAPTIVE", false);
        buttonGroupRouting = new ButtonGroup();
        buttonGroupRouting.add(radioButtonOblivious);
        buttonGroupRouting.add(radioButtonAdaptive);
        JPanel panelRouting = new JPanel(new FlowLayout(FlowLayout.LEFT));
        panelRouting.add(labelRouting);
        panelRouting.add(radioButtonOblivious);
        panelRouting.add(radioButtonAdaptive);
        panelSettings.add(panelRouting);

        // Dimensions
        JPanel panelDimensions = new JPanel(new FlowLayout(FlowLayout.LEFT));
        textFieldWidth = new JTextField("8", 5);
        textFieldHeight = new JTextField("8", 5);
        textFieldNodes = new JTextField("64", 5);
        panelDimensions.add(new JLabel("Width:"));
        panelDimensions.add(textFieldWidth);
        panelDimensions.add(Box.createHorizontalStrut(15));
        panelDimensions.add(new JLabel("Height:"));
        panelDimensions.add(textFieldHeight);
        panelDimensions.add(Box.createHorizontalStrut(15));
        panelDimensions.add(new JLabel("Nodes:"));
        panelDimensions.add(textFieldNodes);
        panelSettings.add(panelDimensions);

        this.add(panelSettings, BorderLayout.NORTH);
    }

    private void initPanelLog() {
        textAreaLog = new JTextArea();
        textAreaLog.setEditable(false);
        textAreaLog.setBackground(Color.BLACK);
        textAreaLog.setForeground(Color.GREEN);
        textAreaLog.setFont(new Font("Consolas", Font.PLAIN, 14));
        JScrollPane scrollPaneLog = new JScrollPane(textAreaLog);
        scrollPaneLog.setBorder(BorderFactory.createLineBorder(Color.GRAY));
        this.add(scrollPaneLog, BorderLayout.CENTER);
    }

    private void initPanelControls() {
        JPanel panelControls = new JPanel();
        panelControls.setBorder(BorderFactory.createEmptyBorder(10, 0, 10, 0));
        buttonRun = new JButton("RUN SIMULATION");
        buttonRun.setFont(new Font("Arial", Font.BOLD, 14));
        buttonRun.addActionListener(this);
        buttonRun.setPreferredSize(new Dimension(200, 40));
        panelControls.add(buttonRun);
        this.add(panelControls, BorderLayout.SOUTH);
    }

    @Override
    public void actionPerformed(ActionEvent event) {
        if (event.getSource() == buttonRun) {
            handleButtonRunAction();
        }
    }

    private void handleButtonRunAction() {
        log("--- Starting Simulation ---");

        // 1. Считывание параметров
        String topology = radioButtonMesh.isSelected() ? "MESH" : "BUTTERFLY";
        String routing = radioButtonOblivious.isSelected() ? "OBLIVIOUS" : "ADAPTIVE";

        int width, height, nodes;
        try {
            width = Integer.parseInt(textFieldWidth.getText().trim());
            height = Integer.parseInt(textFieldHeight.getText().trim());
            nodes = Integer.parseInt(textFieldNodes.getText().trim());
        } catch (NumberFormatException e) {
            log("ERROR: Invalid dimensions!");
            return;
        }

        // 2. Сборка JSON через org.json
        JSONObject configJson = new JSONObject();
        configJson.put("topology", topology);
        configJson.put("routing", routing);
        configJson.put("width", width);
        configJson.put("height", height);
        configJson.put("nodes", nodes);

        String jsonConfig = configJson.toString();
        log("Config JSON: " + jsonConfig);

        // 3. Вызов C++ Initialize
        int resultCode = nativeInitialize(jsonConfig);
        if (resultCode != 0) {
            String errorMsg = nativeGetLastError();
            log("ERROR [C++]: Code " + resultCode + " - " + errorMsg);
            return;
        }

        // 4. Запуск симуляции и получение результата
        String resultJsonStr = nativeRun();
        log("Result JSON: " + resultJsonStr);

        // 5. Парсинг результата через org.json
        try {
            JSONObject resultJson = new JSONObject(resultJsonStr);
            boolean success = resultJson.getBoolean("success");

            if (success) {
                double latency = resultJson.getDouble("latency_avg");
                double throughput = resultJson.getDouble("throughput");
                int delivered = resultJson.getInt("packets_delivered");
                int lost = resultJson.getInt("packets_lost");

                log("✓ SIMULATION SUCCESS");
                log(String.format("  Latency (avg): %.3f cycles", latency));
                log(String.format("  Throughput:    %.2f pkts/cycle", throughput));
                log(String.format("  Packets:       %d delivered, %d lost", delivered, lost));
            } else {
                String error = resultJson.getString("error");
                log("✗ SIMULATION FAILED: " + error);
            }
        } catch (Exception e) {
            log("ERROR parsing result JSON: " + e.getMessage());
        }

        log("-----------------------------");
    }

    private void log(String message) {
        textAreaLog.append(message + "\n");
        textAreaLog.setCaretPosition(textAreaLog.getDocument().getLength());
    }

    public static void main(String[] args) {
        try {
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        } catch (Exception e) {
            e.printStackTrace();
        }
        SwingUtilities.invokeLater(() -> new InterconnectGUI());
    }
}
