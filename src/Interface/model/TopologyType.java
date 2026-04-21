package Interface.model;

public enum TopologyType {
    MESH("MESH"),
    BUTTERFLY("BUTTERFLY");

    private final String value;

    TopologyType(String value) {
        this.value = value;
    }

    public String getValue() {
        return value;
    }

    @Override
    public String toString() {
        return value;
    }
}
