/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package screenmaskerlancherfx;

import java.io.File;
import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.control.ScrollPane;
import javafx.scene.layout.VBox;
import javafx.stage.Stage;
import org.ini4j.Wini;

/**
 *
 * @author paulorlov
 */
public class ScreenMaskerLancherFX extends Application {
    
    @Override
    public void start(Stage stage) throws Exception {
        

        EmptyWindowController awc = new EmptyWindowController();
        FXMLLoader loader = new FXMLLoader(getClass().getResource("EmptyWindow.fxml"));
        loader.setController(awc);
        Parent root = loader.load();
        
        loader = new FXMLLoader(getClass().getResource("MaskedPattern.fxml"));
        loader.setController(awc);
        Parent maskedPattern = loader.load();
        
        loader = new FXMLLoader(getClass().getResource("Stencil.fxml"));
        loader.setController(awc);
        Parent stencil = loader.load();
        
        loader = new FXMLLoader(getClass().getResource("Cursor.fxml"));
        loader.setController(awc);
        Parent cursor = loader.load();
        
        loader = new FXMLLoader(getClass().getResource("UDPServer.fxml"));
        loader.setController(awc);
        Parent serverIP = loader.load();
        
        loader = new FXMLLoader(getClass().getResource("EyeTrackerWindows.fxml"));
        loader.setController(awc);
        Parent eyeTrackerWindows = loader.load();
                
        Scene scene = new Scene(root);   
        ScrollPane sp = (ScrollPane) scene.lookup("#emptyWindowScrollPane");
        VBox content = new VBox();
        sp.setContent(content);
        content.getChildren().addAll(maskedPattern, stencil, cursor, serverIP, eyeTrackerWindows);
        
        stage.setScene(scene);
        stage.show();
        
        File iniFile = new File(System.getProperty("user.dir") + "/options.ini");
        //TODO check exists()
        
        awc.initializeByHand(new Wini(iniFile));
        
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        launch(args);
    }
    
}
