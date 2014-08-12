/******************************************************************************
 * Copyright (C) 2007 Tetsuya Kimata <kimata@acapulco.dyndns.org>
 *
 * All rights reserved.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must
 *    not claim that you wrote the original software. If you use this
 *    software in a product, an acknowledgment in the product
 *    documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must
 *    not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 * $Id: UploadFrame.java 2609 2007-10-27 15:53:25Z svn $
 *****************************************************************************/

package org.dyndns.acapulco;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPasswordField;
import javax.swing.JProgressBar;
import javax.swing.JTextField;

public class UploadFrame extends JFrame {
	private static final long serialVersionUID = 416883618611965749L;

	private final JTextField urlField = new JTextField("http://");
    private final JTextField filePathField = new JTextField();
    private final JTextField commentField = new JTextField();
    private final JPasswordField removePassField = new JPasswordField();
    private final JPasswordField downloadPassField = new JPasswordField();
    private final JProgressBar uploadProgress = new JProgressBar(0, 100);
    private UploadController controller;

    public UploadFrame() {
        super();
        initialize();
    }

    public UploadConfig getConfig() {
        return new UploadConfig(urlField.getText(),
                                filePathField.getText(),
                                commentField.getText(),
                                new String(removePassField.getPassword()),
                                new String(downloadPassField.getPassword()));
    }

    public void showMessage(String message) {
        JOptionPane.showMessageDialog(this, message, "information",
                                      JOptionPane.INFORMATION_MESSAGE);
    }

    public void setController(UploadController aController) {
        controller = aController;
    }

    private void initialize() {
        setJMenuBar(createMenuBar());

        add(createControlPanel(), BorderLayout.CENTER);
        add(uploadProgress, BorderLayout.SOUTH);

        setTitle("UploaderClient");

        addWindowListener(new WindowAdapter() {
                public void windowClosing(WindowEvent e) {
                    System.exit(0);
                }
            });

        pack();
    }

    private JMenuBar createMenuBar() {
        JMenuBar menuBar = new JMenuBar();

        JMenu fileMenu = new JMenu("File");
        fileMenu.setMnemonic('F');
        menuBar.add(fileMenu);

        JMenuItem exitMenu = new JMenuItem("Exit");
        exitMenu.setMnemonic('x');
        exitMenu.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    System.exit(0);
                }
            });
        fileMenu.add(exitMenu);

        return menuBar;
    }

    private JPanel createControlPanel() {
        JPanel panel = new JPanel();

        panel.setLayout(new BorderLayout());
        panel.add(createInputPanel(), BorderLayout.CENTER);

        JButton uploadButton = new JButton("upload");
        uploadButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    controller.startUpload();
                }
            });

        panel.add(uploadButton, BorderLayout.SOUTH);

        return panel;
    }

    private JPanel createInputPanel() {
        JPanel panel = new JPanel();
        GridBagLayout layout = new GridBagLayout();
        GridBagConstraints constraints = new GridBagConstraints();

        panel.setLayout(layout);

        constraints.anchor = GridBagConstraints.WEST;
        constraints.ipadx = 4;

        JLabel urlLabel = new JLabel("url:");
        urlLabel.setLabelFor(urlField);
        constraints.gridx = 1;
        constraints.gridy = 1;
        layout.setConstraints(urlLabel, constraints);
        panel.add(urlLabel);

        urlField.setColumns(40);
        urlField.setMinimumSize(urlField.getPreferredSize());
        constraints.gridx = 2;
        constraints.gridy = 1;
        layout.setConstraints(urlField, constraints);
        panel.add(urlField);

        JLabel fileLabel = new JLabel("file:");
        fileLabel.setLabelFor(filePathField);
        constraints.gridx = 1;
        constraints.gridy = 2;
        constraints.anchor = GridBagConstraints.WEST;
        layout.setConstraints(fileLabel, constraints);
        panel.add(fileLabel);

        JPanel filePathInputPanel = createFileInputPanel();
        constraints.gridx = 2;
        constraints.gridy = 2;
        layout.setConstraints(filePathInputPanel, constraints);
        panel.add(filePathInputPanel);

        JLabel commentLabel = new JLabel("comment:");
        commentLabel.setLabelFor(commentField);
        constraints.gridx = 1;
        constraints.gridy = 3;
        layout.setConstraints(commentLabel, constraints);
        panel.add(commentLabel);

        commentField.setColumns(26);
        commentField.setMinimumSize(commentField.getPreferredSize());
        constraints.gridx = 2;
        constraints.gridy = 3;
        layout.setConstraints(commentField, constraints);
        panel.add(commentField);


        JLabel downloadPassLabel = new JLabel("DL pass:");
        downloadPassLabel.setLabelFor(downloadPassField);
        constraints.gridx = 1;
        constraints.gridy = 4;
        layout.setConstraints(downloadPassLabel, constraints);
        panel.add(downloadPassLabel);

        downloadPassField.setColumns(10);
        downloadPassField.setMinimumSize(downloadPassField.getPreferredSize());
        constraints.gridx = 2;
        constraints.gridy = 4;
        layout.setConstraints(downloadPassField, constraints);
        panel.add(downloadPassField);

        JLabel removePassLabel = new JLabel("RM pass:");
        removePassLabel.setLabelFor(removePassField);
        constraints.gridx = 1;
        constraints.gridy = 5;
        layout.setConstraints(removePassLabel, constraints);
        panel.add(removePassLabel);

        removePassField.setColumns(10);
        removePassField.setMinimumSize(removePassField.getPreferredSize());
        constraints.gridx = 2;
        constraints.gridy = 5;
        layout.setConstraints(removePassField, constraints);
        panel.add(removePassField);

        return panel;
    }

    private JPanel createFileInputPanel() {
        JPanel panel = new JPanel();
        FlowLayout layout = new FlowLayout(FlowLayout.LEFT);

        layout.setHgap(0);
        layout.setVgap(0);
        panel.setLayout(layout);
        filePathField.setColumns(32);
        panel.add(filePathField);

        final JButton fileButton = new JButton("open");
        fileButton.setPreferredSize(new Dimension(fileButton.getPreferredSize().width,
                                                  filePathField.getPreferredSize().height));
        fileButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e){
                    JFileChooser chooser = new JFileChooser();

                    if (chooser.showOpenDialog(fileButton) != JFileChooser.APPROVE_OPTION) {
                        return;
                    }

                    filePathField.setText(chooser.getSelectedFile().getPath());
                }
            });

        panel.add(fileButton);

        return panel;
    }
}

// Local Variables:
// mode: java
// coding: utf-8-dos
// End:
