package Interface.exception;

public class SimulationException extends Exception {
    private final int errorCode;

    public SimulationException(String message) {
        super(message);
        this.errorCode = -1;
    }

    public SimulationException(String message, int errorCode) {
        super(message);
        this.errorCode = errorCode;
    }

    public int getErrorCode() {
        return errorCode;
    }
}
