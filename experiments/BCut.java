import java.io.*;

public final class BCut {
    private static String delimeter = ",";

    public static void main(String[] args) throws Exception {
            String[] field_strs = args[0].split(",");
            int[] fields = new int[field_strs.length];
            int i = 0;
            for (String field: field_strs)
                fields[i++] = Integer.parseInt(field) - 1;
            bcut(fields);
    }

    public static void bcut(int[] fields) throws Exception {
        BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));
        BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(System.out));
        String line, part;
        String[] parts;
        int i, num_fields = fields.length;
        while((line = reader.readLine()) != null) {
            parts = line.split(delimeter);
            i = 0;
            for (int field: fields) {
                if (field >= parts.length)
                    continue;
                part = parts[field];
                writer.write(part, 0, part.length());
                if (++i < num_fields)
                    writer.write(delimeter, 0, 1);
            }
            writer.write("\n", 0, 1);
        }
        reader.close();
        writer.close();
    }
}
