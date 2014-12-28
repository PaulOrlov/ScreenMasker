/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package screenmaskerlancherfx;

/**
 *
 * @author codeding.com community
 */
import java.lang.*;
import java.io.*;
import java.awt.*;
import java.awt.image.*;

public class PGM {

    private String FilePath;
    //pgm imageheader
    private String Type;
    private String Comment;
    private int Columns, Rows, MaxGray;
    //pgm imagedata
    private int[][] Pixels;

    //constructors
    public PGM() {
        FilePath = "";
        Type = "";
        Comment = "";
        Columns = 0;
        Rows = 0;
        MaxGray = 0;
        Pixels = null;
    }

    public PGM(String tpath) {
        FilePath = tpath;
        readImage();
    }

    public PGM(int tColumns, int tRows) {
        FilePath = "";
        Type = "P5";
        Comment = "";
        MaxGray = 255;
        setDimension(tColumns, tRows);
    }

    //get functions
    public String getFilePath() {
        return (FilePath);
    }

    public String getType() {
        return (Type);
    }

    public String getComment() {
        return (Comment);
    }

    public int getColumns() {
        return (Columns);
    }

    public int getRows() {
        return (Rows);
    }

    public int getMaxGray() {
        return (MaxGray);
    }

    public int getPixel(int tr, int tc) {
        return (tr < 0 || tr > Rows - 1 || tc < 0 || tc > Columns - 1 ? 0 : Pixels[tr][tc]);
    }

    public int[][] getNeighbor(int tr, int tc) {
        int neighbor[][] = new int[3][3];

        neighbor[0][0] = getPixel(tr - 1, tc - 1); //NW
        neighbor[0][1] = getPixel(tr - 1, tc); //N
        neighbor[0][2] = getPixel(tr - 1, tc + 1); //NE
        neighbor[1][0] = getPixel(tr, tc - 1); //W
        neighbor[1][1] = getPixel(tr, tc);
        neighbor[1][2] = getPixel(tr, tc + 1); //E
        neighbor[2][0] = getPixel(tr + 1, tc - 1); //SW
        neighbor[2][1] = getPixel(tr + 1, tc); //S
        neighbor[2][2] = getPixel(tr + 1, tc + 1); //SE

        return (neighbor);
    }

    public int getNeighbor(int tr, int tc, int direction) {
        int pval = 0;
        if (direction == Globals.NW) {
            pval = getPixel(tr - 1, tc - 1);
        } else if (direction == Globals.W) {
            pval = getPixel(tr, tc - 1);
        } else if (direction == Globals.SW) {
            pval = getPixel(tr + 1, tc - 1);
        } else if (direction == Globals.S) {
            pval = getPixel(tr + 1, tc);
        } else if (direction == Globals.SE) {
            pval = getPixel(tr + 1, tc + 1);
        } else if (direction == Globals.E) {
            pval = getPixel(tr, tc + 1);
        } else if (direction == Globals.NE) {
            pval = getPixel(tr - 1, tc + 1);
        } else if (direction == Globals.N) {
            pval = getPixel(tr - 1, tc);
        }
        return (pval);
    }

    //set functions
    public void setFilePath(String tFilePath) {
        FilePath = tFilePath;
    }

    public void setType(String tType) {
        Type = tType;
    }

    public void setComment(String tComment) {
        Comment = tComment;
    }

    public void setDimension(int tColumns, int tRows) {
        Rows = tRows;
        Columns = tColumns;
        Pixels = new int[Rows][Columns];
    }

    public void setMaxGray(int tMaxGray) {
        MaxGray = tMaxGray;
    }

    public void setPixel(int tr, int tc, int tval) {
        if (tr < 0 || tr > Rows - 1 || tc < 0 || tc > Columns - 1) {
            return;
        } else {
            Pixels[tr][tc] = tval;
        }
    }

    //methods
    public void readImage() {
        try {
            FileInputStream fin = new FileInputStream(FilePath);

            int c;
            String tstr;

            //read first line of ImageHeader
            tstr = "";
            c = fin.read();
            tstr += (char) c;
            c = fin.read();
            tstr += (char) c;
            Type = tstr;

            //read second line of ImageHeader
            c = fin.read(); //read Lf (linefeed)
            c = fin.read(); //read '#'
            tstr = "";
            boolean iscomment = false;
            while ((char) c == '#') //read comment
            {
                iscomment = true;
                tstr += (char) c;
                while (c != 10 && c != 13) {
                    c = fin.read();
                    tstr += (char) c;
                }
                c = fin.read(); //read next '#'
            }
            if (tstr.equals("") == false) {
                Comment = tstr.substring(0, tstr.length() - 1);
                fin.skip(-1);
            }

            //read third line of ImageHeader
            //read columns
            tstr = "";
            if (iscomment == true) {
                c = fin.read();
            }
            tstr += (char) c;
            while (c != 32 && c != 10 && c != 13) {
                c = fin.read();
                tstr += (char) c;
            }
            tstr = tstr.substring(0, tstr.length() - 1);
            Columns = Integer.parseInt(tstr);

            //read rows
            c = fin.read();
            tstr = "";
            tstr += (char) c;
            while (c != 32 && c != 10 && c != 13) {
                c = fin.read();
                tstr += (char) c;
            }
            tstr = tstr.substring(0, tstr.length() - 1);
            Rows = Integer.parseInt(tstr);

            //read maxgray
            c = fin.read();
            tstr = "";
            tstr += (char) c;
            while (c != 32 && c != 10 && c != 13) {
                c = fin.read();
                tstr += (char) c;
            }
            tstr = tstr.substring(0, tstr.length() - 1);
            MaxGray = Integer.parseInt(tstr);

            //read pixels from ImageData
            Pixels = new int[Rows][Columns];
            for (int tr = 0; tr < Rows; tr++) {
                for (int tc = 0; tc < Columns; tc++) {
                    c = (int) fin.read();
                    setPixel(tr, tc, c);
                }
            }

            fin.close();
        } catch (Exception err) {
            System.out.println("Error: " + err);
            System.exit(-1);
        }
    }

    public void writeImage() {
        try {
            FileOutputStream fout = new FileOutputStream(FilePath);

            //write image header
            //write PGM magic value 'P5'
            String tstr;
            tstr = "P5" + "\n";
            fout.write(tstr.getBytes());

            //write comment
            Comment = "# " + Comment + "\n";
            fout.write(Comment.getBytes());

            //write columns
            tstr = Integer.toString(Columns);
            fout.write(tstr.getBytes());
            fout.write(32); //write blank space

            //write rows
            tstr = Integer.toString(Rows);
            fout.write(tstr.getBytes());
//            fout.write(32); //write blank space
            fout.write("\n".getBytes());

            //write maxgray
            tstr = Integer.toString(MaxGray);
            tstr = tstr + "\n";
            fout.write(tstr.getBytes());

            for (int r = 0; r < Rows; r++) {
                for (int c = 0; c < Columns; c++) {
                    fout.write(getPixel(r, c));
                }
            }

            fout.close();
        } catch (Exception err) {
            System.out.println("Error: " + err);
            System.exit(-1);
        }
    }

    public void writeImageAs(String tFilePath) {
        PGM imgout = new PGM(getColumns(), getRows());

        for (int r = 0; r < getRows(); r++) {
            for (int c = 0; c < getColumns(); c++) {
                imgout.setPixel(r, c, getPixel(r, c));
            }
        }

        imgout.setFilePath(tFilePath);
        imgout.writeImage();
    }

    public BufferedImage getBufferedImage() {
        BufferedImage timg = new BufferedImage(Columns, Rows, BufferedImage.TYPE_INT_RGB);

        for (int r = 0; r < getRows(); r++) {
            for (int c = 0; c < getColumns(); c++) {
                int tgray = getPixel(r, c);
                int trgb = getRGBValue(tgray, tgray, tgray);
                timg.setRGB(c, r, trgb);
            }
        }

        return (timg);
    }

    //static methods
    public static int getRGBValue(int tred, int tgreen, int tblue) {
        return ((tred << 16) + (tgreen << 8) + tblue);
    }

/**
 *
 * @author paul.a.orlov
 */
    static void createTiles(PGM bigPGM, PGM tilePGM) {
        BufferedImage bigImage = bigPGM.getBufferedImage();
        BufferedImage tileImage = tilePGM.getBufferedImage();
        TexturePaint imagePaint = new TexturePaint(tileImage, new Rectangle(0, 0, tilePGM.getColumns(), tilePGM.getRows()));
        Graphics g = bigImage.getGraphics();
        Graphics2D g2d = (Graphics2D)g;
        g2d.setPaint(imagePaint);
        g2d.fill(new Rectangle(0, 0, bigPGM.getColumns(), bigPGM.getRows()));
        for (int r = 0; r < bigPGM.getRows(); r++) {
            for (int c = 0; c < bigPGM.getColumns(); c++) {
                int trgb = bigImage.getRGB(c, r);

//                int alpha = (trgb >> 24) & 0xff;
//                int red = (trgb >> 16) & 0xff;
//                int green = (trgb >> 8) & 0xff;
//                int blue = (trgb) & 0xff;                
//                System.out.println("alpha: " + alpha + "; red: " + red + "; green: " + green + "; blue: " + blue);
                
                bigPGM.setPixel(r, c, (trgb >> 8) & 0xff);
            }
        }
    }
}
