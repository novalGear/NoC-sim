package Interface.gui;

import javax.swing.*;
import java.awt.*;

public class LogPanel extends JPanel {
    private final JTextArea textArea;

    public enum LogLevel {
        INFO(Color.GREEN),
        WARNING(Color.YELLOW),
        ERROR(Color.RED);

        final Color color;
        LogLevel(Color color) { this.color = color; }
    }

    public LogPanel() {
        setLayout(new BorderLayout());
        setBorder(BorderFactory.createTitledBorder("Logs"));

        textArea = new JTextArea();
        textArea.setEditable(false);
        textArea.setBackground(Color.BLACK);
        textArea.setForeground(Color.GREEN);
        textArea.setFont(new Font("Consolas", Font.PLAIN, 12));

        JScrollPane scrollPane = new JScrollPane(textArea);
        scrollPane.setPreferredSize(new Dimension(600, 200));
        add(scrollPane, BorderLayout.CENTER);
    }

    public void log(String message) {
        log(message, LogLevel.INFO);
    }

    public void log(String message, LogLevel level) {
        String prefix = "";
        switch (level) {
            case WARNING: prefix = "[WARN] "; break;
            case ERROR: prefix = "[ERROR] "; break;
            default: prefix = "[INFO] ";
        }

        textArea.setForeground(level.color);
        textArea.append(prefix + message + "\n");
        textArea.setForeground(LogLevel.INFO.color);
        textArea.setCaretPosition(textArea.getDocument().getLength());
    }

    public void clear() {
        textArea.setText("");
    }
}
