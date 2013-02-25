<?php
/**
* @file
* Form element of Multistep form
*/
function form_reg_user($form, &$form_state) {
  // Display page 3 if $form_state['page_num'] == 3
  if (!empty($form_state['page_num']) && $form_state['page_num'] == 3) {
    return multistep_registration_page_three($form, $form_state);
  }
  // Display page 2 if $form_state['page_num'] == 2
  if (!empty($form_state['page_num']) && $form_state['page_num'] == 2) {
    return multistep_registration_page_two($form, $form_state);
  }

  // Otherwise we build page 1.
  $form_state['page_num'] = 1;
  $form['description'] = array(
    '#type' => 'item',
    '#title' => t(''),
  );
  // Account information.
  $form['account'] = array(
    '#type'   => 'container',
    '#weight' => -10,
  );
  $form['account']['mail'] = array(
    '#type' => 'textfield',
    '#title' => t('E-mail address'),
    '#maxlength' => EMAIL_MAX_LENGTH,
    '#description' => t('A valid e-mail address. All e-mails from the system will be sent to this address. The e-mail address is not made public and will only be used if you wish to receive a new password or wish to receive certain news or notifications by e-mail.'),
    '#required' => TRUE,
    '#default_value' => !empty($form_state['values']['mail']) ? $form_state['values']['mail'] : '',
  );
  $form['account']['re_mail'] = array(
    '#type' => 'textfield',
    '#title' => t('Confirm e-mail address'),
    '#maxlength' => EMAIL_MAX_LENGTH,
    '#description' => t('Please re-type your e-mail address to confirm it is accurate.'),
    '#required' => TRUE,
    '#default_value' => !empty($form_state['values']['mail']) ? $form_state['values']['mail'] : '',
  );
  $form['next_1'] = array(
    '#type' => 'submit',
    '#value' => 'Next >>',
    '#submit' => array('form_reg_user_next_1_submit'),
    '#validate' => array('form_reg_user_next_1_validate'),
    '#weight' => 100,
  );
  return $form;
}

/**
 * Returns the form for the second step.
 */
function multistep_registration_page_two($form, &$form_state) {
  $form['description'] = array(
    '#type' => 'item',
    '#title' => t('You have following options of credits. Please choose one'),
  );

  $form['color'] =  array(
    '#type' => 'radios',
    '#title' => t('Avilable Options'),
    '#default_value' => NULL,
    '#options' => array(0 => t('Option 1'), 1 => t('Option 2')),
    '#description' => t('desc will go here'),
    );
    //'#default_value' => !empty($form_state['values']['color']) ? $form_state['values']['color'] : '',
  
  $form['back'] = array(
    '#type' => 'submit',
    '#value' => t('<< Back'),
    '#submit' => array('form_reg_user_page_2_back'),
    '#limit_validation_errors' => array(),
    '#weight' => 100,
  );
  $form['next_2'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
    '#submit' => array('form_reg_user_next_2_submit'),
    '#weight' => 100,
  );
  return $form;
}
/**
* Returns the third page (preview)
*/
function multistep_registration_page_three($form, &$form_state) {
  //return 'this is preview of the form';
  $form['contact_information'] = array(
    '#markup' => '<b>Account Preview:</b><br \><b>Name :</b>' . $form_state['page_values'][1]['name'] . '<br \>',
  );
  $form['back'] = array(
    '#type' => 'submit',
    '#value' => t('<< Back'),
    '#submit' => array('form_reg_user_page_3_back'),
    // We won't bother validating the required 'color' field, since they
    // have to come back to this page to submit anyway.
    '#limit_validation_errors' => array(),
    '#weight' => 100,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Confirm'),
    '#submit' => array('form_reg_user_submit_3_submit'),
    '#weight' => 100,
  );
  //dsm($form);
  dsm($form_state);
  return $form;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Next Button handler
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Validate handler for the next button on first page.
 */
function form_reg_user_next_1_validate($form, &$form_state) {

  if (trim($form_state['values']['mail']) != trim($form_state['values']['re_mail'])) {
    form_set_error('re_mail', t('The e-mail addresses are not same.'));
    }
     
  // Trim whitespace from mail, to prevent confusing 'e-mail not valid'

  $mail = trim($form_state['values']['mail']);
  form_set_value($form['account']['mail'], $mail, $form_state);

  // Validate the e-mail address, and check if it is taken by an existing user.
  if ($error = user_validate_mail($form_state['values']['mail'])) {
    form_set_error('mail', $error);
  }
  elseif ((bool) db_select('users')->fields('users', array('uid'))->condition('mail', db_like($form_state['values']['mail']), 'LIKE')->range(0, 1)->execute()->fetchField()) {
    // Format error message dependent on whether the user is logged in or not.
    if ($GLOBALS['user']->uid) {
      form_set_error('mail', t('The e-mail address %email is already taken.', array('%email' => $form_state['values']['mail'])));
    }
    else {
      form_set_error('mail', t('The e-mail address %email is already registered. <a href="@password">Have you forgotten your password?</a>', array('%email' => $form_state['values']['mail'], '@password' => url('user/password'))));
    }
  }
}
/**
 * Submit handler for first_step() next button.
 *
 * Capture the values from page one and store them away so they can be used
 * at final submit time.
 */
function form_reg_user_next_1_submit($form, &$form_state) {
  // generating username from email
  $new_name = preg_replace('/@.*$/', '', $form_state['values']['mail']);
  // Remove unwanted characters.                                                                                                             
  $new_name = preg_replace('/[^a-zA-Z0-9.-]/', '', $new_name);
  $num = 1;
  // if username generated from email record already exists, append underscore and number eg:(chris_123)                                     
  while((bool) db_select('users')->fields('users', array('uid'))->condition('name', db_like($new_name), 'LIKE')->range(0, 1)->execute()->fetchField()){
    $num++;
    $new_name = $new_name . '_' . $num;
  }
  $form_state['values']['name'] = $new_name;

  $form_state['page_values'][1] = $form_state['values'];

  if (!empty($form_state['page_values'][2])) {
    $form_state['values'] = $form_state['page_values'][2];
  }

  // When form rebuilds, it will look at this to figure which page to build.
  $form_state['page_num'] = 2;
  $form_state['rebuild'] = TRUE;
}
/**
* The page 2 submit handler.
*/
function form_reg_user_next_2_submit($form, &$form_state) {

  // Values are saved for each page.
  // to carry forward to subsequent pages in the form.
  // and we tell FAPI to rebuild the form.
  $form_state['page_values'][2] = $form_state['values'];

  if (!empty($form_state['page_values'][3])) {
    $form_state['values'] = $form_state['page_values'][3];
  }

  // When form rebuilds, it will look at this to figure which page to build.
  $form_state['page_num'] = 3;
  $form_state['rebuild'] = TRUE;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Back button handler
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Back button handler submit handler.
 *
 * Since #limit_validation_errors = array() is set, values from page 2
 * will be discarded. We load the page 1 values instead.
 */
function form_reg_user_page_2_back($form, &$form_state) {
  $form_state['values'] = $form_state['page_values'][1];
  $form_state['page_num'] = 1;
  $form_state['rebuild'] = TRUE;
}

function form_reg_user_page_3_back($form, &$form_state) {
  $form_state['values'] = $form_state['page_values'][2];
  $form_state['page_num'] = 2;
  $form_state['rebuild'] = TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Submit handler
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * The page 3 submit handler.
 *
 * This is the final submit handler. Gather all the data together and output
 * it in a drupal_set_message().
 */

function form_reg_user_submit_3_submit($form, &$form_state) {
  //dsm($form_state);
  $roles = '';
    if (arg(1) == 'employer') {
      $roles = array('5' => 'employer');
    }
    elseif (arg(1) == 'designer') {
      $roles = array('6' => 'designer');
    }
  $edit = array(
    'name' => $form_state['page_values'][1]['name'], 
    'pass' => user_password(),
    'mail' => $form_state['page_values'][1]['mail'],
    'init' => $form_state['page_values'][1]['mail'], 
    'status' => 1, 
    'access' => REQUEST_TIME,
    'roles' => $roles,
  );
  $new_user = user_save(drupal_anonymous_user(), $edit);
  $new_uid = $new_user->uid;
  
  if (arg(1) == 'employer') {
  // clean form values before creating new profile
  profile2_form_submit_cleanup($form, $form_state);
  $profile = profile2_create(array('type' => arg(1), 'uid' => $new_user->uid));
  $profile->field_name = $form_state['page_values'][1]['profile_employer']['field_name'];
  $profile->field_dob = $form_state['page_values'][1]['profile_employer']['field_dob'];
  $profile->field_gender = $form_state['page_values'][1]['profile_employer']['field_gender'];
  $profile->field_name = $form_state['page_values'][1]['profile_employer']['field_name'];
  $profile->field_address = $form_state['page_values'][1]['profile_employer']['field_address'];
  $profile->field_city = $form_state['page_values'][1]['profile_employer']['field_city'];
  $profile->field_contact_no = $form_state['page_values'][1]['profile_employer']['field_contact_no'];
  $profile->field_location = $form_state['page_values'][1]['profile_employer']['field_location'];
  profile2_save($profile);
  }
  
  if (arg(1) == 'designer') {
  // clean form values before creating new profile
  profile2_form_submit_cleanup($form, $form_state);
  $profile = profile2_create(array('type' => arg(1), 'uid' => $new_user->uid));
  $profile->field_name = $form_state['page_values'][1]['profile_designer']['field_name'];
  $profile->field_dob = $form_state['page_values'][1]['profile_designer']['field_dob'];
  $profile->field_gender = $form_state['page_values'][1]['profile_designer']['field_gender'];
  $profile->field_name = $form_state['page_values'][1]['profile_designer']['field_name'];
  $profile->field_address = $form_state['page_values'][1]['profile_designer']['field_address'];
  $profile->field_city = $form_state['page_values'][1]['profile_designer']['field_city'];
  $profile->field_contact_no = $form_state['page_values'][1]['profile_designer']['field_contact_no'];
  $profile->field_location = $form_state['page_values'][1]['profile_designer']['field_location'];
  $profile->field_working_experience = $form_state['page_values'][1]['profile_designer']['field_working_experience'];
  $profile->field_field_availability = $form_state['page_values'][1]['profile_designer']['field_availability'];
  //  $profile->field_school_collection = $form_state['page_values'][1]['profile_designer']['field_school_collection'];
  $profile->field_prefered_working_location_ = $form_state['page_values'][1]['profile_designer']['field_prefered_working_location_'];
  $profile->field_specialization_ = $form_state['page_values'][1]['profile_designer']['field_specialization_'];
  profile2_save($profile);
  }
  
}
function get_profile_detail($new_uid){
  return db_select('profile', 'p')
      ->fields('p')
      ->condition('uid', $new_uid)
      ->execute()
      ->fetchAssoc();
}

// Saving the profile field values.

function save_profile_detail($pid, $profile_fields){
  foreach($profile_fields as $name => $p_field){
    $tablename = 'field_data_' . $name;
    $value_column_name = $name . '_value';
    db_insert($tablename)
      ->fields(array(
        'entity_type' => 'profile2',
        'bundle' => arg(1),
        'entity_id' => $pid,
        'revision_id' => $pid,
        'language' => 'und',
        'delta' => 0,
        $value_column_name => $p_field['und'][0]['value'],
    ))
    ->execute();
  }
}