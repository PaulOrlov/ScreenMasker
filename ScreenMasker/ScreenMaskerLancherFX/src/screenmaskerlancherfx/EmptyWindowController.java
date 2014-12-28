package screenmaskerlancherfx;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.ResourceBundle;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.application.Platform;
import javafx.beans.property.BooleanProperty;
import javafx.beans.property.SimpleBooleanProperty;
import javafx.beans.property.SimpleStringProperty;
import javafx.beans.property.StringProperty;
import javafx.embed.swing.SwingFXUtils;
import javafx.event.ActionEvent;
import javafx.event.Event;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.CheckBox;
import javafx.scene.control.ColorPicker;
import javafx.scene.control.TextField;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.paint.Color;
import javafx.stage.FileChooser;
import org.ini4j.Ini;
import org.ini4j.Wini;

/**
 *
 * @author paulorlov
 */
public class EmptyWindowController implements Initializable {
    
    private StringProperty stecilPicturePath_ = new SimpleStringProperty(), 
            color_ = new SimpleStringProperty(), 
            maskedPicturePath_ = new SimpleStringProperty(), 
            serverIP_ = new SimpleStringProperty(), 
            listenServerIP_ = new SimpleStringProperty(), 
            shiftX_ = new SimpleStringProperty(), 
            shiftY_ = new SimpleStringProperty(), 
            serverPort_ = new SimpleStringProperty(),
            listenServerPort_ = new SimpleStringProperty();
    
    private BooleanProperty isEyeWindowShow_ = new SimpleBooleanProperty(), 
            isEyePositionWindowShow_ = new SimpleBooleanProperty(),
            isCalibrate_ = new SimpleBooleanProperty();
    
    @FXML
    private TextField stecilPicturePath;
    
    @FXML
    private TextField stecilSize;
        
    @FXML
    private ImageView stencilImageView;
    
    @FXML
    private ColorPicker stencilColorPicker;
            
    private String texureColor;
    
    @FXML
    private TextField maskedPicturePath;
    
    @FXML
    private TextField maskedPictureSize;
        
    @FXML
    private ImageView maskedPictureImageView;
    
    @FXML
    private TextField shiftX;
    
    @FXML
    private TextField shiftY;
    
    @FXML
    private TextField serverIP;
    
    @FXML
    private TextField serverPort;
    
    @FXML
    private TextField listenServerIP;
    
    @FXML
    private TextField listenServerPort;
    
    @FXML
    private CheckBox isEyeWindowShow;
    
    @FXML
    private CheckBox isEyePositionWindowShow;

    @FXML
    private CheckBox isCalibrate;
    
    @FXML
    private void handleStencilButtonAction(ActionEvent event) {
        FileChooser fileChooser = new FileChooser();
        FileChooser.ExtensionFilter extFilter = new FileChooser.ExtensionFilter("PGM files (*.pgm)", "*.pgm");
        fileChooser.getExtensionFilters().add(extFilter);
        File file = fileChooser.showOpenDialog(null);
        
        if(file != null){
            stecilPicturePath.setText(file.getPath());
            PGM pgmImg = new PGM(stecilPicturePath.getText());
            Image image = SwingFXUtils.toFXImage(pgmImg.getBufferedImage(), null);
            stencilImageView.setImage(image);
            stecilSize.setText("w: " + image.getWidth() + " h: " + image.getHeight());
        }
    }
    
    @FXML
    private void handleMaskedButtonAction(ActionEvent event) {
        FileChooser fileChooser = new FileChooser();
        FileChooser.ExtensionFilter extFilter = new FileChooser.ExtensionFilter("PGM files (*.pgm)", "*.pgm");
        fileChooser.getExtensionFilters().add(extFilter);
        File file = fileChooser.showOpenDialog(null);
        
        if(file != null){
            maskedPicturePath.setText(file.getPath());
            PGM pgmImg = new PGM(maskedPicturePath.getText());
            Image image = SwingFXUtils.toFXImage(pgmImg.getBufferedImage(), null);
            maskedPictureImageView.setImage(image);
            maskedPictureSize.setText("w: " + image.getWidth() + " h: " + image.getHeight());
        }
    }
    
    @FXML
    private void handleRunButtonAction(ActionEvent event){
        
        Thread thread = new Thread(new Runnable() {

            @Override
            public void run() {
                try {

                    File iniFile = new File(System.getProperty("user.dir") + "/options.ini");
                    if(iniFile.exists()){
                        Wini ini = new Wini(iniFile);
                        ini.put("Base", "stecilPicturePath", stecilPicturePath_.getValue());
                        ini.put("Base", "maskedPicturePath", maskedPicturePath_.getValue());
                        ini.put("Base", "color", color_.getValue().substring(1));
                        ini.put("Base", "shiftX", shiftX_.getValue());
                        ini.put("Base", "shiftY", shiftY_.getValue());
                        ini.put("Base", "serverIP", serverIP_.getValue());
                        ini.put("Base", "serverPort", serverPort_.getValue());
                        ini.put("Base", "listenUDPIP", listenServerIP_.getValue());
                        ini.put("Base", "listenUDPPort", listenServerPort_.getValue());
                        ini.put("Base", "isEyeWindowShow", isEyeWindowShow_.getValue());
                        ini.put("Base", "isEyePositionWindowShow", isEyePositionWindowShow_.getValue());
                        ini.put("Base", "isCalibrate", isCalibrate_.getValue());
                        ini.store();
                        
                        List<String> command = new ArrayList<String>();
                        File tmpFile = new File(getClass().getProtectionDomain().getCodeSource().getLocation().getPath());
                        System.out.println(tmpFile.getParentFile().getParentFile().getPath() + "\\ScreenMasker.exe");
                        command.add(tmpFile.getParentFile().getParentFile().getPath() + "\\ScreenMasker.exe");
                        ProcessBuilder builder = new ProcessBuilder(command);
                        Map<String, String> environ = builder.environment();
                        environ.put("PATH", "/windows;/windows/system32;/winnt");
                        final Process godot = builder.start();
                        Platform.exit();
                    } else {
                        System.err.println("Working Directory = " + System.getProperty("user.dir") + "NO");
                    }
                    
                } catch (IOException ex) {
                    Logger.getLogger(EmptyWindowController.class.getName()).log(Level.SEVERE, null, ex);
                }
            }
        });
        thread.start();
    }
    
    public static String toRGBCode( Color color ){
        return String.format( "#%02X%02X%02X",
            (int)( color.getRed() * 255 ),
            (int)( color.getGreen() * 255 ),
            (int)( color.getBlue() * 255 ) );
    }
    
    public void initializeByHand(Wini ini) {
        maskedPicturePath_.bindBidirectional(maskedPicturePath.textProperty());
        stecilPicturePath_.bindBidirectional(stecilPicturePath.textProperty());
        stencilColorPicker.setOnAction(new EventHandler() {
            public void handle(Event t) {
                color_.setValue(toRGBCode(stencilColorPicker.getValue()));
            }
        });
        serverIP_.bindBidirectional(serverIP.textProperty());
        serverPort_.bindBidirectional(serverPort.textProperty());
        
        listenServerIP_.bindBidirectional(listenServerIP.textProperty());
        listenServerPort_.bindBidirectional(listenServerPort.textProperty());
        
        shiftX_.bindBidirectional(shiftX.textProperty());
        shiftY_.bindBidirectional(shiftY.textProperty());
        
        isEyeWindowShow_.bindBidirectional(isEyeWindowShow.selectedProperty());
        isEyePositionWindowShow_.bindBidirectional(isEyePositionWindowShow.selectedProperty());
        isCalibrate_.bindBidirectional(isCalibrate.selectedProperty());
        
        Ini.Section base = ini.get("Base");
        //TODO check exists()
        
        String maskedPicturePathTmp = base.get("maskedPicturePath");
        PGM pgmImg = new PGM(maskedPicturePathTmp);
        Image image = SwingFXUtils.toFXImage(pgmImg.getBufferedImage(), null);
        maskedPictureImageView.setImage(image);
        maskedPictureSize.setText("w: " + image.getWidth() + " h: " + image.getHeight());
        maskedPicturePath_.setValue(maskedPicturePathTmp);
        
        String stencilPathTmp = base.get("stecilPicturePath");
        PGM pgmImgSt = new PGM(stencilPathTmp);
        Image imageSt = SwingFXUtils.toFXImage(pgmImgSt.getBufferedImage(), null);
        stencilImageView.setImage(imageSt);
        stecilSize.setText("w: " + imageSt.getWidth() + " h: " + imageSt.getHeight());
        stecilPicturePath_.setValue(stencilPathTmp);
        
        stencilColorPicker.setValue(Color.web("#"+base.get("color")));
        color_.setValue(toRGBCode(stencilColorPicker.getValue()));
        shiftX_.setValue(base.get("shiftX"));
        shiftY_.setValue(base.get("shiftY"));
        serverIP_.setValue(base.get("serverIP"));
        serverPort_.setValue(base.get("serverPort"));
        listenServerIP_.setValue(base.get("listenUDPIP"));
        listenServerPort_.setValue(base.get("listenUDPPort"));
        
        isEyeWindowShow_.setValue(Boolean.valueOf(base.get("isEyeWindowShow")));
        isEyePositionWindowShow_.setValue(Boolean.valueOf(base.get("isEyePositionWindowShow")));
        isCalibrate_.setValue(Boolean.valueOf(base.get("isCalibrate")));
    }
    
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        
    }       

}
