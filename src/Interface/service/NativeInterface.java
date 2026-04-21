package Interface.service;

import Interface.exception.SimulationException;

public class NativeInterface {
    private static NativeInterface instance;
    private boolean initialized = false;

    static {
        try {
            System.loadLibrary("JNIdll");
        } catch (UnsatisfiedLinkError e) {
            System.err.println("Failed to load native library: " + e.getMessage());
            throw new RuntimeException("Cannot load JNIdll library", e);
        }
    }

    private native int nativeInitialize(String jsonConfig);
    private native String nativeRun();
    private native String nativeGetLastError();

    private NativeInterface() {}

    public static synchronized NativeInterface getInstance() {
        if (instance == null) {
            instance = new NativeInterface();
        }
        return instance;
    }

    public void initialize(String jsonConfig) throws SimulationException {
        int resultCode = nativeInitialize(jsonConfig);
        if (resultCode != 0) {
            String errorMsg = nativeGetLastError();
            initialized = false;
            throw new SimulationException(
                String.format("Initialization failed (code %d): %s", resultCode, errorMsg),
                resultCode
            );
        }
        initialized = true;
    }

    public String runSimulation() throws SimulationException {
        if (!initialized) {
            throw new SimulationException("Simulation not initialized");
        }

        String result = nativeRun();
        if (result == null || result.isEmpty()) {
            throw new SimulationException("No result returned from simulation");
        }

        return result;
    }

    public boolean isInitialized() {
        return initialized;
    }

    public void reset() {
        initialized = false;
    }
}
