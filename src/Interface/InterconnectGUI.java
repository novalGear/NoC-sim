import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

public class InterconnectGUI extends JFrame implements ActionListener {

    private native int nativeInitialize(String jsonConfig);
    private native int nativeRun();
    private native String nativeGetLastError();

    // TODO:
    static {
        // System.load("/mnt/c/Users/agniy/NoC-sim/src/libJNIdll.so");
        System.loadLibrary("JNIdll");
    }

    // === 1. ОБЪЯВЛЕНИЕ ПЕРЕМЕННЫХ (ПОЛЕЙ КЛАССА) ===

    // --- Элементы Topology ---
    private ButtonGroup buttonGroupTopology;
    private JRadioButton radioButtonMesh;
    private JRadioButton radioButtonButterfly;

    // --- Элементы Routing ---
    private ButtonGroup buttonGroupRouting;
    private JRadioButton radioButtonOblivious;
    private JRadioButton radioButtonAdaptive;

    // --- Поля ввода размеров ---
    private JTextField textFieldWidth;
    private JTextField textFieldHeight;
    private JTextField textFieldNodes;

    // --- Логирование ---
    private JTextArea textAreaLog;

    // --- Кнопки управления ---
    private JButton buttonRun;

    // === 2. КОНСТРУКТОР ===
    public InterconnectGUI() {
        super("Interconnect Simulator GUI");

        // Базовые настройки окна
        this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        this.setSize(750, 650);
        this.setLayout(new BorderLayout());

        // Вызываем методы инициализации частей интерфейса
        initPanelSettings();
        initPanelLog();
        initPanelControls();

        this.setVisible(true);
        log("System initialized. Ready.");
        log("Enter dimensions and click Run.");
    }

    // === 3. МЕТОДЫ ИНИЦИАЛИЗАЦИИ ===

    // Метод создает верхнюю панель с настройками
    private void initPanelSettings() {
        // Главная панель настроек: 3 строки (Topology, Routing, Dimensions)
        JPanel panelSettings = new JPanel(new GridLayout(3, 1));
        panelSettings.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));

        // --- Строка 1: Topology ---
        JLabel labelTopology = new JLabel("Topology:");
        radioButtonMesh = new JRadioButton("Mesh", true);
        radioButtonButterfly = new JRadioButton("Butterfly", false);

        buttonGroupTopology = new ButtonGroup();
        buttonGroupTopology.add(radioButtonMesh);
        buttonGroupTopology.add(radioButtonButterfly);

        JPanel panelTopology = new JPanel(new FlowLayout(FlowLayout.LEFT));
        panelTopology.add(labelTopology);
        panelTopology.add(radioButtonMesh);
        panelTopology.add(radioButtonButterfly);

        panelSettings.add(panelTopology);

        // --- Строка 2: Routing ---
        JLabel labelRouting = new JLabel("Routing:");
        radioButtonOblivious = new JRadioButton("Oblivious", true);
        radioButtonAdaptive = new JRadioButton("Adaptive", false);

        buttonGroupRouting = new ButtonGroup();
        buttonGroupRouting.add(radioButtonOblivious);
        buttonGroupRouting.add(radioButtonAdaptive);

        JPanel panelRouting = new JPanel(new FlowLayout(FlowLayout.LEFT));
        panelRouting.add(labelRouting);
        panelRouting.add(radioButtonOblivious);
        panelRouting.add(radioButtonAdaptive);

        panelSettings.add(panelRouting);

        // --- Строка 3: Dimensions ---
        JPanel panelDimensions = new JPanel(new FlowLayout(FlowLayout.LEFT));

        JLabel labelWidth = new JLabel("Width:");
        textFieldWidth = new JTextField("8", 5);

        JLabel labelHeight = new JLabel("Height:");
        textFieldHeight = new JTextField("8", 5);

        JLabel labelNodes = new JLabel("Nodes:");
        textFieldNodes = new JTextField("64", 5);

        panelDimensions.add(labelWidth);
        panelDimensions.add(textFieldWidth);
        panelDimensions.add(Box.createHorizontalStrut(15));
        panelDimensions.add(labelHeight);
        panelDimensions.add(textFieldHeight);
        panelDimensions.add(Box.createHorizontalStrut(15));
        panelDimensions.add(labelNodes);
        panelDimensions.add(textFieldNodes);

        panelSettings.add(panelDimensions);

        this.add(panelSettings, BorderLayout.NORTH);
    }

    // Метод создает центральную область логов
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

    // Метод создает нижнюю панель с кнопками
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

    // === 4. ОБРАБОТКА СОБЫТИЙ ===

    @Override
    public void actionPerformed(ActionEvent event) {
        if (event.getSource() == buttonRun) {
            handleButtonRunAction();
        }
    }

    private void handleButtonRunAction() {
        log("--- Starting Simulation ---");

        // 1. Считываем и валидируем параметры
        String topology = radioButtonMesh.isSelected() ? "MESH" : "BUTTERFLY";
        String routing = radioButtonOblivious.isSelected() ? "OBLIVIOUS" : "ADAPTIVE";

        int width, height, nodes;
        try {
            width = Integer.parseInt(textFieldWidth.getText().trim());
            height = Integer.parseInt(textFieldHeight.getText().trim());
            nodes = Integer.parseInt(textFieldNodes.getText().trim());
        } catch (NumberFormatException e) {
            log("ERROR: Invalid dimensions format!");
            return;
        }

        // 2. Формируем JSON-конфигурацию
        String jsonConfig = buildConfigJson(topology, routing, width, height, nodes);
        log("Config prepared: " + jsonConfig); // Для отладки

        // 3. Вызываем C++ бэкенд
        int resultCode = nativeInitialize(jsonConfig);

        if (resultCode != 0) {
            // Получаем детальное сообщение об ошибке из C++
            String errorMsg = nativeGetLastError();
            log("ERROR [C++]: Code " + resultCode + " - " + errorMsg);
            return;
        }

        // 4. Запускаем симуляцию
        int runResult = nativeRun();
        if (runResult == 0) {
            log("SUCCESS: Simulation completed.");
        } else {
            log("ERROR [C++]: Run failed with code " + runResult);
        }

        log("-----------------------------");
    }

    // Вспомогательный метод для сборки JSON
    private String buildConfigJson(String topology, String routing, int width, int height, int nodes) {
        // Вариант с org.json (рекомендуется)
        org.json.JSONObject config = new org.json.JSONObject();
        config.put("topology", topology);
        config.put("routing", routing);
        config.put("width", width);
        config.put("height", height);
        config.put("nodes", nodes);
        return config.toString();

        // Вариант без библиотек (если не хотите зависимости):
        // return String.format("{\"topology\":\"%s\",\"routing\":\"%s\",\"width\":%d,\"height\":%d,\"nodes\":%d}",
        //         topology, routing, width, height, nodes);
    }

    // Вспомогательный метод для вывода в лог
    private void log(String message) {
        textAreaLog.append(message + "\n");
        textAreaLog.setCaretPosition(textAreaLog.getDocument().getLength());
    }

    // === 5. ТОЧКА ВХОДА ===
    public static void main(String[] args) {
        try {
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        } catch (Exception exception) {
            exception.printStackTrace();
        }

        SwingUtilities.invokeLater(() -> new InterconnectGUI());
    }
}
