package Interface.model;

public enum RoutingType {
    STATIC("STATIC"),
    OBLIVIOUS("OBLIVIOUS"),
    ADAPTIVE("ADAPTIVE");

    private final String value;

    RoutingType(String value) {
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
