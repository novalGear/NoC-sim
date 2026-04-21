package Interface;

import Interface.gui.InterconnectFrame;
import javax.swing.*;

public class InterconnectApp {
    public static void main(String[] args) {
        try {
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        } catch (Exception e) {
            e.printStackTrace();
        }

        SwingUtilities.invokeLater(() -> {
            InterconnectFrame frame = new InterconnectFrame();
            frame.setVisible(true);
        });
    }
}
