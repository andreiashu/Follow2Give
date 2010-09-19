<?php

twitter_integration_include();

function twitter_goto_twitter_login($return_url = NULL) {
  $twitterObj = twitter_get_EpiTwitter();

  drupal_goto(twitter_generate_authorizeUrl($return_url));
}

function twitter_generate_authorizeUrl($return_url = NULL) {
  $twitterObj = twitter_get_EpiTwitter();
  if($return_url) {
    return $twitterObj->getAuthorizeUrl(null,array('oauth_callback' => url($return_url, array('absolute' => TRUE))));
  }
  else {
    return $twitterObj->getAuthorizeUrl();
  }
}

function twitter_get_EpiTwitter($oauth_token = NULL, $oauth_token_secret = NULL) {
  static $twitterObj;

  if(!is_object($twitterObj)) {
    $twitterObj = new EpiTwitter(CONSUMER_KEY, CONSUMER_SECRET, $oauth_token, $oauth_token_secret);
  }

  return $twitterObj;
}
/*
function twitter_oauth_load_user() {
  dpm('twitter_oauth_load_user');
  if(!isset($_GET['oauth_token'])) {
    return FALSE;
  }

  $twitterObj = twitter_get_EpiTwitter();

  try {
    $twitterObj->setToken($_GET['oauth_token']);
    $token = $twitterObj->getAccessToken(array('oauth_verifier' => $_GET['oauth_verifier']));
    $account = user_external_load($token->user_id);

    if (!$account) {
      return FALSE;
    }

    $account->twitter_token = $token;

  } catch (Exception $e) {
    print_r($e);
    drupal_set_message("Twitter API integration error [twitter_oauth_load_user]: " . $e->getMessage());
    return FALSE;
  }

  return $account;
}

function twitter_oauth_save_user($role_id = NULL) {
  dpm('twitter_oauth_save_user');
  if(!isset($_GET['oauth_token'])) {
    return FALSE;
  }

  $twitterObj = twitter_get_EpiTwitter();

  try {
    $twitterObj->setToken($_GET['oauth_token']);
    $token = $twitterObj->getAccessToken(array('oauth_verifier' => $_GET['oauth_verifier']));
    //dpm($token);
    //$twitterObj->setToken($token->oauth_token, $token->oauth_token_secret);

    $account = twitter_user_save($token, $role_id);

    $account->twitter_token = $token;
  } catch (Exception $e) {
    print_r($e);
    drupal_set_message("Twitter API integration error [twitter_oauth_save_user]: " . $e->getMessage());
    return FALSE;
  }

  return $account;

}
*/

function twitter_return_load_user($role_id = NULL) {
  $twitterObj = twitter_get_EpiTwitter();

  try {
    $twitterObj->setToken($_GET['oauth_token']);
    $token = $twitterObj->getAccessToken(array('oauth_verifier' => $_GET['oauth_verifier']));
    $account = user_external_load($token->user_id);

    if (!isset($account->uid)) { // current user
      $account = twitter_user_save($token, $role_id);
    }

    $account->twitter_token = $token;

  } catch (EpiException $e) {
    drupal_set_message("Twitter API integration error: " . $e->getMessage());
    return FALSE;
  }

  return $account;
}

function twitter_user_save($token, $role_id = NULL) {
  if($account = user_load(array('name' => $token->screen_name))) {
    return $account;
  }

  $edit = array(
    'name' => $token->screen_name,
    'pass' => user_password(),
    'init' => $token->screen_name,
    'status' => 1,
    "authname_twitter" => $token->user_id,
    'access' => time(),
  );
  if(!empty($role_id)) {
    $edit['roles'] = array(
      $role_id => $role_id,
    );
  }

  $account = user_save('', $edit);

  return $account;
}

function twitter_integration_include() {
  static $called = FALSE;
  if($called) {
    return;
  }
  $called = TRUE;
  $path = drupal_get_path('module', 'twitter_integration') . '/includes/twitter-async';

  require_once($path . '/' . 'EpiCurl.php');
  require_once($path . '/' . 'EpiOAuth.php');
  require_once($path . '/' . 'EpiTwitter.php');
}

function twitter_user_load_oauth_token($uid) {
  return db_fetch_object(db_query("SELECT * FROM {f2g_twitter_tokens} WHERE uid = %d", $uid));
}

function twitter_user_save_oauth_token($uid, $token) {
  db_query("INSERT IGNORE INTO {f2g_twitter_tokens} (oauth_token, oauth_token_secret, screen_name, uid) VALUES ('%s', '%s', '%s', %d)", $token->oauth_token, $token->oauth_token_secret, $token->screen_name, $uid);
}

/**
 * !!!! TODO: user cursor for people that have more than 100 friends !!!!
 */
function twitter_user_friends($twitterObj, $screen_name) {
  try {
    $response = $twitterObj->get('/statuses/friends.json', array('screen_name' => $response->screen_name));
    return $response->response;
  }
  catch(Exception $e) {
    drupal_set_message(t('An error ocurred while getting friends: !error', array('!error' => $e->getMessage())), 'error');
  }
}