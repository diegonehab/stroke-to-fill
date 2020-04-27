package br.impa.app;

import java.awt.Graphics2D;
import java.awt.geom.GeneralPath;
import java.awt.geom.PathIterator;
import java.awt.geom.AffineTransform;
import java.awt.BasicStroke;
import java.awt.Shape;
import java.util.Locale;
import java.util.LinkedHashMap;
import java.util.Map;
import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.parser.*;
import java.io.FileReader;
import java.io.InputStreamReader;
import java.util.Iterator;
import java.util.Map;


class App {

    static int basicCap(String cap) {
        if (cap.equals("stroke_cap.round")) {
            return BasicStroke.CAP_ROUND;
        } else if (cap.equals("stroke_cap.square")) {
            return BasicStroke.CAP_SQUARE;
        } else {
            return BasicStroke.CAP_BUTT;
        }
    }

    static int basicJoin(String join) {
        if (join.equals("stroke_join.round")) {
            return BasicStroke.JOIN_ROUND;
        } else if (join.equals("stroke_join.bevel")) {
            return BasicStroke.JOIN_BEVEL;
        } else {
            return BasicStroke.JOIN_MITER;
        }
    }

    public static void main(String args[]) throws Exception {

        // typecasting obj to JSONObject
        JSONObject jo = (JSONObject) new JSONParser().parse(new InputStreamReader(System.in));

        // getting firstName and lastName
        double width = (double) jo.get("width");
        int cap = basicCap((String) jo.get("cap"));
        int join = basicJoin((String) jo.get("join"));
        double miterlimit = (double) jo.get("miterlimit");
        double dashoffset = width * ((double) jo.get("dashoffset"));
        Boolean resetsonmove = (Boolean) jo.get("resetsonmove");
        JSONArray jp = (JSONArray) jo.get("path");
        GeneralPath p = new GeneralPath();
        for (int i = 0; i < jp.size(); i++) {
            JSONArray cmd = (JSONArray) jp.get(i);
            String c = (String) cmd.get(0);
            if (c.equals("M")) {
                double x0 = (double) cmd.get(1);
                double y0 = (double) cmd.get(2);
                p.moveTo((float) x0, (float) y0);
            } else if (c.equals("L")) {
                double x0 = (double) cmd.get(1);
                double y0 = (double) cmd.get(2);
                p.lineTo((float) x0, (float) y0);
            } else if (c.equals("Q")) {
                double x0 = (double) cmd.get(1);
                double y0 = (double) cmd.get(2);
                double x1 = (double) cmd.get(3);
                double y1 = (double) cmd.get(4);
                p.quadTo((float) x0, (float) y0,
                        (float) x1, (float) y1);
            } else if (c.equals("C")) {
                double x0 = (double) cmd.get(1);
                double y0 = (double) cmd.get(2);
                double x1 = (double) cmd.get(3);
                double y1 = (double) cmd.get(4);
                double x2 = (double) cmd.get(5);
                double y2 = (double) cmd.get(6);
                p.curveTo((float) x0, (float) y0,
                        (float) x1, (float) y1,
                        (float) x2, (float) y2);
            } else if (c.equals("Z")) {
                p.closePath();
            }
        }
        BasicStroke st;
        JSONArray jdashes = (JSONArray) jo.get("dasharray");
        if (jdashes.size() > 0) {
            float dashes[] = new float[jdashes.size()];
            for (int i = 0; i < jdashes.size(); i++) {
                dashes[i] = (float) (width * (double) jdashes.get(i));
            }
            st = new BasicStroke((float) width, cap, join, (float) miterlimit,
                dashes, (float) dashoffset);
        } else {
            st = new BasicStroke((float) width, cap, join, (float) miterlimit);
        }
        Locale.setDefault(new Locale ("en", "US"));
        Shape s = st.createStrokedShape(p);
        float coords[] = new float[6];
        PathIterator it = s.getPathIterator(new AffineTransform());
        while (!it.isDone()) {
            int what = it.currentSegment(coords);
            switch (what) {
                case PathIterator.SEG_CLOSE:
                    System.out.println("Z ");
                    break;
                case PathIterator.SEG_MOVETO:
                    System.out.printf("M %f %f ", coords[0], coords[1]);
                    break;
                case PathIterator.SEG_LINETO:
                    System.out.printf("L %f %f ", coords[0], coords[1]);
                    break;
                case PathIterator.SEG_QUADTO:
                    System.out.printf("Q %f %f %f %f ", coords[0], coords[1], coords[2], coords[3]);
                    break;
                case PathIterator.SEG_CUBICTO:
                    System.out.printf("C %f %f %f %f %f %f ", coords[0], coords[1],
                            coords[2], coords[3], coords[4], coords[5]);
                    break;
            }
            it.next();
        }

    }
}
