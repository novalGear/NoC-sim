package Interface.gui;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionListener;

public class ControlPanel extends JPanel {
    private final JButton runButton;
    private final JButton clearButton;
    private final JProgressBar progressBar;
    private boolean running = false;

    public ControlPanel() {
        setLayout(new FlowLayout(FlowLayout.CENTER, 10, 10));
        setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));

        runButton = new JButton("RUN SIMULATION");
        runButton.setFont(new Font("Arial", Font.BOLD, 14));
        runButton.setPreferredSize(new Dimension(180, 40));

        clearButton = new JButton("CLEAR LOGS");
        clearButton.setFont(new Font("Arial", Font.PLAIN, 12));
        clearButton.setPreferredSize(new Dimension(120, 40));

        progressBar = new JProgressBar();
        progressBar.setIndeterminate(false);
        progressBar.setPreferredSize(new Dimension(200, 25));
        progressBar.setVisible(false);

        add(runButton);
        add(clearButton);
        add(progressBar);
    }

    public void addRunListener(ActionListener listener) {
        runButton.addActionListener(listener);
    }

    public void addClearListener(ActionListener listener) {
        clearButton.addActionListener(listener);
    }

    public void setRunning(boolean running) {
        this.running = running;
        runButton.setEnabled(!running);
        runButton.setText(running ? "SIMULATION RUNNING..." : "RUN SIMULATION");
        progressBar.setVisible(running);
        progressBar.setIndeterminate(running);
    }

    public boolean isRunning() {
        return running;
    }
}
